/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
  generator.cpp
*/
#include "generator.h"

#include "codemarker.h"
#include "config.h"
#include "doc.h"
#include "editdistance.h"
#include "loggingcategory.h"
#include "node.h"
#include "openedlist.h"
#include "qdocdatabase.h"
#include "quoter.h"
#include "separator.h"
#include "tokenizer.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>

#ifndef QT_BOOTSTRAPPED
#    include "QtCore/qurl.h"
#endif

QT_BEGIN_NAMESPACE

Generator *Generator::currentGenerator_;
QStringList Generator::exampleDirs;
QStringList Generator::exampleImgExts;
QMap<QString, QMap<QString, QString>> Generator::fmtLeftMaps;
QMap<QString, QMap<QString, QString>> Generator::fmtRightMaps;
QVector<Generator *> Generator::generators;
QStringList Generator::imageDirs;
QStringList Generator::imageFiles;
QMap<QString, QStringList> Generator::imgFileExts;
QString Generator::outDir_;
QString Generator::outSubdir_;
QStringList Generator::outFileNames_;
QSet<QString> Generator::outputFormats;
QHash<QString, QString> Generator::outputPrefixes;
QHash<QString, QString> Generator::outputSuffixes;
QString Generator::project_;
QStringList Generator::scriptDirs;
QStringList Generator::scriptFiles;
QStringList Generator::styleDirs;
QStringList Generator::styleFiles;
bool Generator::noLinkErrors_ = false;
bool Generator::autolinkErrors_ = false;
bool Generator::redirectDocumentationToDevNull_ = false;
bool Generator::qdocSingleExec_ = false;
bool Generator::useOutputSubdirs_ = true;
QmlTypeNode *Generator::qmlTypeContext_ = nullptr;

static QRegExp tag("</?@[^>]*>");
static QLatin1String amp("&amp;");
static QLatin1String gt("&gt;");
static QLatin1String lt("&lt;");
static QLatin1String quot("&quot;");

/*!
  Constructs the generator base class. Prepends the newly
  constructed generator to the list of output generators.
  Sets a pointer to the QDoc database singleton, which is
  available to the generator subclasses.
 */
Generator::Generator()
    : inLink_(false),
      inContents_(false),
      inSectionHeading_(false),
      inTableHeader_(false),
      threeColumnEnumValueTable_(true),
      showInternal_(false),
      singleExec_(false),
      numTableRows_(0)
{
    qdb_ = QDocDatabase::qdocDB();
    generators.prepend(this);
}

/*!
  Destroys the generator after removing it from the list of
  output generators.
 */
Generator::~Generator()
{
    generators.removeAll(this);
}

void Generator::appendFullName(Text &text, const Node *apparentNode, const Node *relative,
                               const Node *actualNode)
{
    if (actualNode == nullptr)
        actualNode = apparentNode;
    text << Atom(Atom::LinkNode, CodeMarker::stringForNode(actualNode))
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
         << Atom(Atom::String, apparentNode->plainFullName(relative))
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
}

void Generator::appendFullName(Text &text, const Node *apparentNode, const QString &fullName,
                               const Node *actualNode)
{
    if (actualNode == nullptr)
        actualNode = apparentNode;
    text << Atom(Atom::LinkNode, CodeMarker::stringForNode(actualNode))
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << Atom(Atom::String, fullName)
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
}

void Generator::appendFullNames(Text &text, const NodeList &nodes, const Node *relative)
{
    int index = 0;
    for (const auto &node : nodes) {
        appendFullName(text, node, relative);
        text << comma(index++, nodes.count());
    }
}

/*!
  Append the signature for the function named in \a node to
  \a text, so that is is a link to the documentation for that
  function.
 */
void Generator::appendSignature(Text &text, const Node *node)
{
    text << Atom(Atom::LinkNode, CodeMarker::stringForNode(node))
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
         << Atom(Atom::String, node->signature(false, true))
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
}

/*!
  Generate a bullet list of function signatures. The function
  nodes are in \a nodes. It uses the \a relative node and the
  \a marker for the generation.
 */
void Generator::signatureList(const NodeList &nodes, const Node *relative, CodeMarker *marker)
{
    Text text;
    int count = 0;
    text << Atom(Atom::ListLeft, QString("bullet"));
    for (const auto &node : nodes) {
        text << Atom(Atom::ListItemNumber, QString::number(++count));
        text << Atom(Atom::ListItemLeft, QString("bullet"));
        appendSignature(text, node);
        text << Atom(Atom::ListItemRight, QString("bullet"));
    }
    text << Atom(Atom::ListRight, QString("bullet"));
    generateText(text, relative, marker);
}

int Generator::appendSortedNames(Text &text, const ClassNode *cn, const QVector<RelatedClass> &rc)
{
    QMap<QString, Text> classMap;
    for (const auto &relatedClass : rc) {
        ClassNode *rcn = relatedClass.node_;
        if (rcn && rcn->isInAPI()) {
            Text className;
            appendFullName(className, rcn, cn);
            classMap[className.toString().toLower()] = className;
        }
    }

    int index = 0;
    const QStringList classNames = classMap.keys();
    for (const auto &className : classNames) {
        text << classMap[className];
        text << comma(index++, classNames.count());
    }
    return index;
}

int Generator::appendSortedQmlNames(Text &text, const Node *base, const NodeList &subs)
{
    QMap<QString, Text> classMap;

    for (const auto sub : subs) {
        Text text;
        if (!base->isQtQuickNode() || !sub->isQtQuickNode()
            || (base->logicalModuleName() == sub->logicalModuleName())) {
            appendFullName(text, sub, base);
            classMap[text.toString().toLower()] = text;
        }
    }

    int index = 0;
    const QStringList names = classMap.keys();
    for (const auto &name : names) {
        text << classMap[name];
        text << comma(index++, names.count());
    }
    return index;
}

/*!
  For debugging qdoc.
 */
void Generator::writeOutFileNames()
{
    QFile files("outputlist.txt");
    if (!files.open(QFile::WriteOnly))
        return;
    QTextStream filesout(&files);
    const auto names = outFileNames_;
    for (const auto &file : names) {
        filesout << file << "\n";
    }
}

/*!
  Creates the file named \a fileName in the output directory
  and returns a QFile pointing to this file. In particular,
  this method deals with errors when opening the file:
  the returned QFile is always valid and can be written to.

  \sa beginFilePage()
 */
QFile *Generator::openSubPageFile(const Node *node, const QString &fileName)
{
    QString path = outputDir() + QLatin1Char('/');
    if (Generator::useOutputSubdirs() && !node->outputSubdirectory().isEmpty()
        && !outputDir().endsWith(node->outputSubdirectory())) {
        path += node->outputSubdirectory() + QLatin1Char('/');
    }
    path += fileName;

    auto outPath = redirectDocumentationToDevNull_ ? QStringLiteral("/dev/null") : path;
    auto outFile = new QFile(outPath);
    if (!redirectDocumentationToDevNull_ && outFile->exists()) {
        node->location().error(
                tr("Output file already exists; overwriting %1").arg(outFile->fileName()));
    }
    if (!outFile->open(QFile::WriteOnly)) {
        node->location().fatal(tr("Cannot open output file '%1'").arg(outFile->fileName()));
    }
    qCDebug(lcQdoc, "Writing: %s", qPrintable(path));
    outFileNames_ << fileName;
    return outFile;
}

/*!
  Creates the file named \a fileName in the output directory.
  Attaches a QTextStream to the created file, which is written
  to all over the place using out(). This function does not
  store the \a fileName in the \a node as the output file name.

  \sa beginSubPage()
 */
void Generator::beginFilePage(const Node *node, const QString &fileName)
{
    QFile *outFile = openSubPageFile(node, fileName);
    QTextStream *out = new QTextStream(outFile);
#ifndef QT_NO_TEXTCODEC
    if (outputCodec)
        out->setCodec(outputCodec);
#endif
    outStreamStack.push(out);
}

/*!
 Creates the file named \a fileName in the output directory.
 Attaches a QTextStream to the created file, which is written
 to all over the place using out(). This function calls another
 function, \c beginFilePage(), which is really just most of what
 this function used to contain. We needed a different version
 that doesn't store the \a fileName in the \a node as the output
 file name.

 \sa beginFilePage()
*/
void Generator::beginSubPage(const Node *node, const QString &fileName)
{
    beginFilePage(node, fileName);
    const_cast<Node *>(node)->setOutputFileName(fileName);
}

/*!
  Flush the text stream associated with the subpage, and
  then pop it off the text stream stack and delete it.
  This terminates output of the subpage.
 */
void Generator::endSubPage()
{
    outStreamStack.top()->flush();
    delete outStreamStack.top()->device();
    delete outStreamStack.pop();
}

/*
  the code below is effectively equivalent to:
  input.replace(QRegExp("[^A-Za-z0-9]+"), " ");
  input = input.trimmed();
  input.replace(QLatin1Char(' '), QLatin1Char('-'));
  input = input.toLower();
  as this function accounted for ~8% of total running time
  we optimize a bit...
*/
static void transmogrify(QString &input, QString &output)
{
    // +5 prevents realloc in fileName() below
    output.reserve(input.size() + 5);
    bool begun = false;
    for (int i = 0; i != input.size(); ++i) {
        QChar c = input.at(i);
        uint u = c.unicode();
        if (u >= 'A' && u <= 'Z')
            u += 'a' - 'A';
        if ((u >= 'a' && u <= 'z') || (u >= '0' && u <= '9')) {
            output += QLatin1Char(u);
            begun = true;
        } else if (begun) {
            output += QLatin1Char('-');
            begun = false;
        }
    }
    while (output.endsWith(QLatin1Char('-')))
        output.chop(1);
}

QString Generator::fileBase(const Node *node) const
{
    if (!node->isPageNode() && !node->isCollectionNode())
        node = node->parent();

    if (node->hasFileNameBase())
        return node->fileNameBase();

    QString base;
    if (node->isCollectionNode()) {
        base = node->name() + outputSuffix(node);
        if (base.endsWith(".html"))
            base.truncate(base.length() - 5);

        if (node->isQmlModule())
            base.append("-qmlmodule");
        else if (node->isJsModule())
            base.append("-jsmodule");
        else if (node->isModule())
            base.append("-module");
        // Why not add "-group" for group pages?
    } else if (node->isTextPageNode()) {
        base = node->name();
        if (base.endsWith(".html"))
            base.truncate(base.length() - 5);

        if (node->isExample()) {
            QString modPrefix(node->physicalModuleName());
            if (modPrefix.isEmpty()) {
                modPrefix = project_;
            }
            base.prepend(modPrefix.toLower() + QLatin1Char('-'));
        }
        if (node->isExample()) {
            base.append(QLatin1String("-example"));
        }
    } else if (node->isQmlType() || node->isQmlBasicType() || node->isJsType()
               || node->isJsBasicType()) {
        base = node->name();
        /*
          To avoid file name conflicts in the html directory,
          we prepend a prefix (by default, "qml-") and an optional suffix
          to the file name. The suffix, if one exists, is appended to the
          module name.
        */
        if (!node->logicalModuleName().isEmpty()
            && (!node->logicalModule()->isInternal() || showInternal_))
            base.prepend(node->logicalModuleName() + outputSuffix(node) + QLatin1Char('-'));

        base.prepend(outputPrefix(node));
    } else if (node->isProxyNode()) {
        base = node->name();
        base.append("-proxy");
    } else {
        const Node *p = node;
        forever {
            const Node *pp = p->parent();
            base.prepend(p->name());
            if (pp == nullptr || pp->name().isEmpty() || pp->isTextPageNode())
                break;
            base.prepend(QLatin1Char('-'));
            p = pp;
        }
        if (node->isNamespace() && !node->name().isEmpty()) {
            const NamespaceNode *ns = static_cast<const NamespaceNode *>(node);
            if (!ns->isDocumentedHere()) {
                base.append(QLatin1String("-sub-"));
                base.append(ns->tree()->camelCaseModuleName());
            }
        }
    }

    QString res;
    transmogrify(base, res);
    Node *n = const_cast<Node *>(node);
    n->setFileNameBase(res);
    return res;
}

/*!
  Constructs an href link from an example file name, which
  is a path to the example file. If \a fileExtension is
  empty (default value), retrieve the file extension from
  the generator.
 */
QString Generator::linkForExampleFile(const QString &path, const Node *parent,
                                      const QString &fileExt)
{
    QString link = path;
    QString modPrefix(parent->physicalModuleName());
    if (modPrefix.isEmpty())
        modPrefix = project_;
    link.prepend(modPrefix.toLower() + QLatin1Char('-'));

    QString res;
    transmogrify(link, res);
    res.append(QLatin1Char('.'));
    res.append(fileExt);
    if (fileExt.isEmpty())
        res.append(fileExtension());
    return res;
}

/*!
    Helper function to construct a title for a file or image page
    included in an example.
*/
QString Generator::exampleFileTitle(const ExampleNode *relative, const QString &fileName)
{
    QString suffix;
    if (relative->files().contains(fileName))
        suffix = QLatin1String(" Example File");
    else if (relative->images().contains(fileName))
        suffix = QLatin1String(" Image File");
    else
        return suffix;

    return fileName.mid(fileName.lastIndexOf(QLatin1Char('/')) + 1) + suffix;
}

/*!
  If the \a node has a URL, return the URL as the file name.
  Otherwise, construct the file name from the fileBase() and
  either the provided \a extension or fileExtension(), and
  return the constructed name.
 */
QString Generator::fileName(const Node *node, const QString &extension) const
{
    if (!node->url().isEmpty())
        return node->url();

    QString name = fileBase(node) + QLatin1Char('.');
    return extension.isNull() ? name + fileExtension() : name + extension;
}

QString Generator::cleanRef(const QString &ref)
{
    QString clean;

    if (ref.isEmpty())
        return clean;

    clean.reserve(ref.size() + 20);
    const QChar c = ref[0];
    const uint u = c.unicode();

    if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9')) {
        clean += c;
    } else if (u == '~') {
        clean += "dtor.";
    } else if (u == '_') {
        clean += "underscore.";
    } else {
        clean += QLatin1Char('A');
    }

    for (int i = 1; i < ref.length(); i++) {
        const QChar c = ref[i];
        const uint u = c.unicode();
        if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || u == '-'
            || u == '_' || u == ':' || u == '.') {
            clean += c;
        } else if (c.isSpace()) {
            clean += QLatin1Char('-');
        } else if (u == '!') {
            clean += "-not";
        } else if (u == '&') {
            clean += "-and";
        } else if (u == '<') {
            clean += "-lt";
        } else if (u == '=') {
            clean += "-eq";
        } else if (u == '>') {
            clean += "-gt";
        } else if (u == '#') {
            clean += QLatin1Char('#');
        } else {
            clean += QLatin1Char('-');
            clean += QString::number(static_cast<int>(u), 16);
        }
    }
    return clean;
}

QMap<QString, QString> &Generator::formattingLeftMap()
{
    return fmtLeftMaps[format()];
}

QMap<QString, QString> &Generator::formattingRightMap()
{
    return fmtRightMaps[format()];
}

/*!
  Returns the full document location.
 */
QString Generator::fullDocumentLocation(const Node *node, bool useSubdir)
{
    if (node == nullptr)
        return QString();
    if (!node->url().isEmpty())
        return node->url();

    QString parentName;
    QString anchorRef;
    QString fdl;

    /*
      If the useSubdir parameter is set, then the output is
      being sent to subdirectories of the output directory.
      Prepend the subdirectory name + '/' to the result.
     */
    if (useSubdir) {
        fdl = node->outputSubdirectory();
        if (!fdl.isEmpty())
            fdl.append(QLatin1Char('/'));
    }
    if (node->isNamespace()) {
        /*
          The root namespace has no name - check for this before creating
          an attribute containing the location of any documentation.
        */
        if (!fileBase(node).isEmpty())
            parentName = fileBase(node) + QLatin1Char('.') + currentGenerator()->fileExtension();
        else
            return QString();
    } else if (node->isQmlType() || node->isQmlBasicType() || node->isJsType()
               || node->isJsBasicType()) {
        QString fb = fileBase(node);
        if (fb.startsWith(outputPrefix(node)))
            return fb + QLatin1Char('.') + currentGenerator()->fileExtension();
        else {
            QString mq;
            if (!node->logicalModuleName().isEmpty()) {
                mq = node->logicalModuleName().replace(QChar('.'), QChar('-'));
                mq = mq.toLower() + QLatin1Char('-');
            }
            return fdl + outputPrefix(node) + mq + fileBase(node) + QLatin1Char('.')
                    + currentGenerator()->fileExtension();
        }
    } else if (node->isTextPageNode() || node->isCollectionNode()) {
        parentName = fileBase(node) + QLatin1Char('.') + currentGenerator()->fileExtension();
    } else if (fileBase(node).isEmpty())
        return QString();

    Node *parentNode = nullptr;

    if ((parentNode = node->parent())) {
        // use the parent's name unless the parent is the root namespace
        if (!node->parent()->isNamespace() || !node->parent()->name().isEmpty())
            parentName = fullDocumentLocation(node->parent());
    }

    switch (node->nodeType()) {
    case Node::Class:
    case Node::Struct:
    case Node::Union:
    case Node::Namespace:
    case Node::Proxy:
        parentName = fileBase(node) + QLatin1Char('.') + currentGenerator()->fileExtension();
        break;
    case Node::Function: {
        const FunctionNode *fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::JsSignal:
        case FunctionNode::QmlSignal:
            anchorRef = QLatin1Char('#') + node->name() + "-signal";
            break;
        case FunctionNode::JsSignalHandler:
        case FunctionNode::QmlSignalHandler:
            anchorRef = QLatin1Char('#') + node->name() + "-signal-handler";
            break;
        case FunctionNode::JsMethod:
        case FunctionNode::QmlMethod:
            anchorRef = QLatin1Char('#') + node->name() + "-method";
            break;
        default:
            if (fn->isDtor())
                anchorRef = "#dtor." + fn->name().mid(1);
            else if (fn->hasOneAssociatedProperty() && fn->doc().isEmpty())
                return fullDocumentLocation(fn->firstAssociatedProperty());
            else if (fn->overloadNumber() > 0)
                anchorRef = QLatin1Char('#') + cleanRef(fn->name()) + QLatin1Char('-')
                        + QString::number(fn->overloadNumber());
            else
                anchorRef = QLatin1Char('#') + cleanRef(fn->name());
            break;
        }
        break;
    }
    /*
      Use node->name() instead of fileBase(node) as
      the latter returns the name in lower-case. For
      HTML anchors, we need to preserve the case.
    */
    case Node::Enum:
        anchorRef = QLatin1Char('#') + node->name() + "-enum";
        break;
    case Node::TypeAlias:
        anchorRef = QLatin1Char('#') + node->name() + "-alias";
        break;
    case Node::Typedef: {
        const TypedefNode *tdef = static_cast<const TypedefNode *>(node);
        if (tdef->associatedEnum()) {
            return fullDocumentLocation(tdef->associatedEnum());
        }
        anchorRef = QLatin1Char('#') + node->name() + "-typedef";
        break;
    }
    case Node::Property:
        anchorRef = QLatin1Char('#') + node->name() + "-prop";
        break;
    case Node::JsProperty:
    case Node::QmlProperty:
        if (node->isAttached())
            anchorRef = QLatin1Char('#') + node->name() + "-attached-prop";
        else
            anchorRef = QLatin1Char('#') + node->name() + "-prop";
        break;
    case Node::Variable:
        anchorRef = QLatin1Char('#') + node->name() + "-var";
        break;
    case Node::JsType:
    case Node::QmlType:
    case Node::Page:
    case Node::Group:
    case Node::HeaderFile:
    case Node::Module:
    case Node::JsModule:
    case Node::QmlModule: {
        parentName = fileBase(node);
        parentName.replace(QLatin1Char('/'), QLatin1Char('-'))
                .replace(QLatin1Char('.'), QLatin1Char('-'));
        parentName += QLatin1Char('.') + currentGenerator()->fileExtension();
    } break;
    default:
        break;
    }

    if (!node->isClassNode() && !node->isNamespace()) {
        if (node->isObsolete())
            parentName.replace(QLatin1Char('.') + currentGenerator()->fileExtension(),
                               "-obsolete." + currentGenerator()->fileExtension());
    }

    return fdl + parentName.toLower() + anchorRef;
}

void Generator::generateAlsoList(const Node *node, CodeMarker *marker)
{
    QVector<Text> alsoList = node->doc().alsoList();
    supplementAlsoList(node, alsoList);

    if (!alsoList.isEmpty()) {
        Text text;
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD) << "See also "
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);

        for (int i = 0; i < alsoList.size(); ++i)
            text << alsoList.at(i) << separator(i, alsoList.size());

        text << Atom::ParaRight;
        generateText(text, node, marker);
    }
}

const Atom *Generator::generateAtomList(const Atom *atom, const Node *relative, CodeMarker *marker,
                                        bool generate, int &numAtoms)
{
    while (atom != nullptr) {
        if (atom->type() == Atom::FormatIf) {
            int numAtoms0 = numAtoms;
            bool rightFormat = canHandleFormat(atom->string());
            atom = generateAtomList(atom->next(), relative, marker, generate && rightFormat,
                                    numAtoms);
            if (atom == nullptr)
                return nullptr;

            if (atom->type() == Atom::FormatElse) {
                ++numAtoms;
                atom = generateAtomList(atom->next(), relative, marker, generate && !rightFormat,
                                        numAtoms);
                if (atom == nullptr)
                    return nullptr;
            }

            if (atom->type() == Atom::FormatEndif) {
                if (generate && numAtoms0 == numAtoms) {
                    relative->location().warning(
                            tr("Output format %1 not handled %2").arg(format()).arg(outFileName()));
                    Atom unhandledFormatAtom(Atom::UnhandledFormat, format());
                    generateAtomList(&unhandledFormatAtom, relative, marker, generate, numAtoms);
                }
                atom = atom->next();
            }
        } else if (atom->type() == Atom::FormatElse || atom->type() == Atom::FormatEndif) {
            return atom;
        } else {
            int n = 1;
            if (generate) {
                n += generateAtom(atom, relative, marker);
                numAtoms += n;
            }
            while (n-- > 0)
                atom = atom->next();
        }
    }
    return nullptr;
}

/*!
  Generate the body of the documentation from the qdoc comment
  found with the entity represented by the \a node.
 */
void Generator::generateBody(const Node *node, CodeMarker *marker)
{
    const FunctionNode *fn = node->isFunction() ? static_cast<const FunctionNode *>(node) : nullptr;
    if (!node->hasDoc() && !node->hasSharedDoc()) {
        /*
          Test for special function, like a destructor or copy constructor,
          that has no documentation.
        */
        if (fn) {
            if (fn->isDtor()) {
                Text text;
                text << "Destroys the instance of ";
                text << fn->parent()->name() << ".";
                if (fn->isVirtual())
                    text << " The destructor is virtual.";
                out() << "<p>";
                generateText(text, node, marker);
                out() << "</p>";
            } else if (fn->isCtor()) {
                Text text;
                text << "Default constructs an instance of ";
                text << fn->parent()->name() << ".";
                out() << "<p>";
                generateText(text, node, marker);
                out() << "</p>";
            } else if (fn->isCCtor()) {
                Text text;
                text << "Copy constructor.";
                out() << "<p>";
                generateText(text, node, marker);
                out() << "</p>";
            } else if (fn->isMCtor()) {
                Text text;
                text << "Move-copy constructor.";
                out() << "<p>";
                generateText(text, node, marker);
                out() << "</p>";
            } else if (fn->isCAssign()) {
                Text text;
                text << "Copy-assignment operator.";
                out() << "<p>";
                generateText(text, node, marker);
                out() << "</p>";
            } else if (fn->isMAssign()) {
                Text text;
                text << "Move-assignment operator.";
                out() << "<p>";
                generateText(text, node, marker);
                out() << "</p>";
            } else if (!node->isWrapper() && !node->isMarkedReimp()) {
                if (!fn->isIgnored()) // undocumented functions added by Q_OBJECT
                    node->location().warning(
                            tr("No documentation for '%1'").arg(node->plainSignature()));
            }
        } else if (!node->isWrapper() && !node->isMarkedReimp()) {
            // Don't require documentation of things defined in Q_GADGET
            if (node->name() != QLatin1String("QtGadgetHelper"))
                node->location().warning(
                        tr("No documentation for '%1'").arg(node->plainSignature()));
        }
    } else if (!node->isSharingComment()) {
        // Reimplements clause and type alias info precede body text
        if (fn && !fn->overridesThis().isEmpty())
            generateReimplementsClause(fn, marker);
        else if (node->isTypeAlias())
            generateAddendum(node, TypeAlias, marker, false);

        if (!generateText(node->doc().body(), node, marker)) {
            if (node->isMarkedReimp())
                return;
        }

        if (fn) {
            if (fn->isQmlSignal())
                generateAddendum(node, QmlSignalHandler, marker);
            if (fn->isPrivateSignal())
                generateAddendum(node, PrivateSignal, marker);
            if (fn->isInvokable())
                generateAddendum(node, Invokable, marker);
            if (fn->hasAssociatedProperties())
                generateAddendum(node, AssociatedProperties, marker);
        }

        // Generate warnings
        if (node->isEnumType()) {
            const EnumNode *enume = static_cast<const EnumNode *>(node);

            QSet<QString> definedItems;
            const QVector<EnumItem> &items = enume->items();
            for (const auto &item : items)
                definedItems.insert(item.name());

            const auto &documentedItemList = enume->doc().enumItemNames();
            QSet<QString> documentedItems(documentedItemList.cbegin(), documentedItemList.cend());
            const QSet<QString> allItems = definedItems + documentedItems;
            if (allItems.count() > definedItems.count()
                || allItems.count() > documentedItems.count()) {
                for (const auto &it : allItems) {
                    if (!definedItems.contains(it)) {
                        QString details;
                        QString best = nearestName(it, definedItems);
                        if (!best.isEmpty() && !documentedItems.contains(best))
                            details = tr("Maybe you meant '%1'?").arg(best);

                        node->doc().location().warning(tr("No such enum item '%1' in %2")
                                                               .arg(it)
                                                               .arg(node->plainFullName()),
                                                       details);
                    } else if (!documentedItems.contains(it)) {
                        node->doc().location().warning(tr("Undocumented enum item '%1' in %2")
                                                               .arg(it)
                                                               .arg(node->plainFullName()));
                    }
                }
            }
        } else if (fn) {
            const QSet<QString> declaredNames = fn->parameters().getNames();
            const QSet<QString> documentedNames = fn->doc().parameterNames();
            if (declaredNames != documentedNames) {
                for (const auto &name : declaredNames) {
                    if (!documentedNames.contains(name)) {
                        if (fn->isActive() || fn->isPreliminary()) {
                            if (!fn->isMarkedReimp() && !fn->isOverload()) {
                                fn->doc().location().warning(tr("Undocumented parameter '%1' in %2")
                                                                     .arg(name)
                                                                     .arg(node->plainFullName()));
                            }
                        }
                    }
                }
                for (const auto &name : documentedNames) {
                    if (!declaredNames.contains(name)) {
                        QString best = nearestName(name, declaredNames);
                        QString details;
                        if (!best.isEmpty())
                            details = tr("Maybe you meant '%1'?").arg(best);
                        fn->doc().location().warning(tr("No such parameter '%1' in %2")
                                                             .arg(name)
                                                             .arg(fn->plainFullName()),
                                                     details);
                    }
                }
            }
            /*
              This return value check should be implemented
              for all functions with a return type.
              mws 13/12/2018
            */
            if (!fn->isObsolete() && fn->returnsBool() && !fn->isMarkedReimp()
                && !fn->isOverload()) {
                if (!fn->doc().body().contains("return"))
                    node->doc().location().warning(
                            tr("Undocumented return value "
                               "(hint: use 'return' or 'returns' in the text"));
            }
        }
    }
    generateRequiredLinks(node, marker);
}

/*!
  Generates either a link to the project folder for example \a node, or a list
  of links files/images if 'url.examples config' variable is not defined.

  Does nothing for non-example nodes.
*/
void Generator::generateRequiredLinks(const Node *node, CodeMarker *marker)
{
    if (!node->isExample())
        return;

    const ExampleNode *en = static_cast<const ExampleNode *>(node);
    QString exampleUrl = Config::instance().getString(CONFIG_URL + Config::dot + CONFIG_EXAMPLES);

    if (exampleUrl.isEmpty()) {
        if (!en->noAutoList()) {
            generateFileList(en, marker, false); // files
            generateFileList(en, marker, true); // images
        }
    } else {
        generateLinkToExample(en, marker, exampleUrl);
    }
}

/*!
  Generates an external link to the project folder for example \a node.
  The path to the example replaces a placeholder '\1' character if
  one is found in the \a baseUrl string. If no such placeholder is found,
  the path is appended to \a baseUrl, after a '/' character if \a baseUrl did
  not already end in one.
*/
void Generator::generateLinkToExample(const ExampleNode *en, CodeMarker *marker,
                                      const QString &baseUrl)
{
    QString exampleUrl(baseUrl);
    QString link;
#ifndef QT_BOOTSTRAPPED
    link = QUrl(exampleUrl).host();
#endif
    if (!link.isEmpty())
        link.prepend(" @ ");
    link.prepend("Example project");

    const QLatin1Char separator('/');
    const QLatin1Char placeholder('\1');
    if (!exampleUrl.contains(placeholder)) {
        if (!exampleUrl.endsWith(separator))
            exampleUrl += separator;
        exampleUrl += placeholder;
    }

    // Construct a path to the example; <install path>/<example name>
    QString pathRoot = en->doc().metaTagMap().value(QLatin1String("installpath"));
    if (pathRoot.isEmpty())
        pathRoot = Config::instance().getString(CONFIG_EXAMPLESINSTALLPATH);
    QStringList path = QStringList() << pathRoot << en->name();
    path.removeAll({});

    Text text;
    text << Atom::ParaLeft
         << Atom(Atom::Link, exampleUrl.replace(placeholder, path.join(separator)))
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << Atom(Atom::String, link)
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << Atom::ParaRight;

    generateText(text, nullptr, marker);
}

void Generator::addImageToCopy(const ExampleNode *en, const QString &file)
{
    QDir dirInfo;
    QString userFriendlyFilePath;
    const QString prefix("/images/used-in-examples/");
    QString srcPath = Config::findFile(en->location(), QStringList(), exampleDirs, file,
                                       exampleImgExts, &userFriendlyFilePath);
    outFileNames_ << prefix.mid(1) + userFriendlyFilePath;
    userFriendlyFilePath.truncate(userFriendlyFilePath.lastIndexOf('/'));
    QString imgOutDir = outDir_ + prefix + userFriendlyFilePath;
    if (!dirInfo.mkpath(imgOutDir))
        en->location().fatal(tr("Cannot create output directory '%1'").arg(imgOutDir));
    Config::copyFile(en->location(), srcPath, file, imgOutDir);
}

/*!
  This function is called when the documentation for an example is
  being formatted. It outputs a list of files for the example, which
  can be the example's source files or the list of images used by the
  example. The images are copied into a subtree of
  \c{...doc/html/images/used-in-examples/...}
*/
void Generator::generateFileList(const ExampleNode *en, CodeMarker *marker, bool images)
{
    Text text;
    OpenedList openedList(OpenedList::Bullet);
    QString tag;
    QStringList paths;
    Atom::AtomType atomType = Atom::ExampleFileLink;

    if (images) {
        paths = en->images();
        tag = "Images:";
        atomType = Atom::ExampleImageLink;
    } else { // files
        paths = en->files();
        tag = "Files:";
    }
    std::sort(paths.begin(), paths.end(), Generator::comparePaths);

    text << Atom::ParaLeft << tag << Atom::ParaRight;
    text << Atom(Atom::ListLeft, openedList.styleString());

    QString path;
    for (const auto &file : qAsConst(paths)) {
        if (images) {
            if (!file.isEmpty())
                addImageToCopy(en, file);
        } else {
            generateExampleFilePage(en, file, marker);
        }

        openedList.next();
        text << Atom(Atom::ListItemNumber, openedList.numberString())
             << Atom(Atom::ListItemLeft, openedList.styleString()) << Atom::ParaLeft
             << Atom(atomType, file) << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << file
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << Atom::ParaRight
             << Atom(Atom::ListItemRight, openedList.styleString());
        path = file;
    }
    text << Atom(Atom::ListRight, openedList.styleString());
    if (!paths.isEmpty())
        generateText(text, en, marker);
}

void Generator::generateInheritedBy(const ClassNode *classe, CodeMarker *marker)
{
    if (!classe->derivedClasses().isEmpty()) {
        Text text;
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
             << "Inherited by: " << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);

        appendSortedNames(text, classe, classe->derivedClasses());
        text << Atom::ParaRight;
        generateText(text, classe, marker);
    }
}

void Generator::generateInherits(const ClassNode *classe, CodeMarker *marker)
{
    if (!classe->baseClasses().isEmpty()) {
        Text text;
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
             << "Inherits: " << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);

        int index = 0;
        const QVector<RelatedClass> &baseClasses = classe->baseClasses();
        for (const auto &cls : baseClasses) {
            if (cls.node_) {
                appendFullName(text, cls.node_, classe);

                if (cls.access_ == Node::Protected) {
                    text << " (protected)";
                } else if (cls.access_ == Node::Private) {
                    text << " (private)";
                }
                text << separator(index++, classe->baseClasses().count());
            }
        }
        text << Atom::ParaRight;
        generateText(text, classe, marker);
    }
}

/*!
  Recursive writing of HTML files from the root \a node.
 */
void Generator::generateDocumentation(Node *node)
{
    if (!node->url().isNull())
        return;
    if (node->isIndexNode())
        return;
    if (node->isInternal() && !showInternal_)
        return;
    if (node->isExternalPage())
        return;

    /*
      Obtain a code marker for the source file.
     */
    CodeMarker *marker = CodeMarker::markerForFileName(node->location().filePath());

    if (node->parent() != nullptr) {
        if (node->isCollectionNode()) {
            /*
              A collection node collects: groups, C++ modules,
              QML modules or JavaScript modules. Testing for a
              CollectionNode must be done before testing for a
              TextPageNode because a CollectionNode is a PageNode
              at this point.

              Don't output an HTML page for the collection
              node unless the \group, \module, \qmlmodule or
              \jsmodule command was actually seen by qdoc in
              the qdoc comment for the node.

              A key prerequisite in this case is the call to
              mergeCollections(cn). We must determine whether
              this group, module, QML module, or JavaScript
              module has members in other modules. We know at
              this point that cn's members list contains only
              members in the current module. Therefore, before
              outputting the page for cn, we must search for
              members of cn in the other modules and add them
              to the members list.
            */
            CollectionNode *cn = static_cast<CollectionNode *>(node);
            if (cn->wasSeen()) {
                qdb_->mergeCollections(cn);
                beginSubPage(node, fileName(node));
                generateCollectionNode(cn, marker);
                endSubPage();
            } else if (cn->isGenericCollection()) {
                // Currently used only for the module's related orphans page
                // but can be generalized for other kinds of collections if
                // other use cases pop up.
                QString name = cn->name().toLower();
                name.replace(QChar(' '), QString("-"));
                QString filename =
                        cn->tree()->physicalModuleName() + "-" + name + "." + fileExtension();
                beginSubPage(node, filename);
                generateGenericCollectionPage(cn, marker);
                endSubPage();
            }
        } else if (node->isTextPageNode()) {
            beginSubPage(node, fileName(node));
            generatePageNode(static_cast<PageNode *>(node), marker);
            endSubPage();
        } else if (node->isAggregate()) {
            if ((node->isClassNode() || node->isHeader() || node->isNamespace())
                && node->docMustBeGenerated()) {
                beginSubPage(node, fileName(node));
                generateCppReferencePage(static_cast<Aggregate *>(node), marker);
                endSubPage();
            } else if (node->isQmlType() || node->isJsType()) {
                beginSubPage(node, fileName(node));
                QmlTypeNode *qcn = static_cast<QmlTypeNode *>(node);
                generateQmlTypePage(qcn, marker);
                endSubPage();
            } else if (node->isQmlBasicType() || node->isJsBasicType()) {
                beginSubPage(node, fileName(node));
                QmlBasicTypeNode *qbtn = static_cast<QmlBasicTypeNode *>(node);
                generateQmlBasicTypePage(qbtn, marker);
                endSubPage();
            } else if (node->isProxyNode()) {
                beginSubPage(node, fileName(node));
                generateProxyPage(static_cast<Aggregate *>(node), marker);
                endSubPage();
            }
        }
    }

    if (node->isAggregate()) {
        Aggregate *aggregate = static_cast<Aggregate *>(node);
        const NodeList &children = aggregate->childNodes();
        for (auto *node : children) {
            if (node->isPageNode() && !node->isPrivate())
                generateDocumentation(node);
        }
    }
}

/*!
  Generate a list of maintainers in the output
 */
void Generator::generateMaintainerList(const Aggregate *node, CodeMarker *marker)
{
    QStringList sl = getMetadataElements(node, "maintainer");

    if (!sl.isEmpty()) {
        Text text;
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
             << "Maintained by: " << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);

        for (int i = 0; i < sl.size(); ++i)
            text << sl.at(i) << separator(i, sl.size());

        text << Atom::ParaRight;
        generateText(text, node, marker);
    }
}

/*!
  Output the "Inherit by" list for the QML element,
  if it is inherited by any other elements.
 */
void Generator::generateQmlInheritedBy(const QmlTypeNode *qcn, CodeMarker *marker)
{
    if (qcn) {
        NodeList subs;
        QmlTypeNode::subclasses(qcn, subs);
        if (!subs.isEmpty()) {
            Text text;
            text << Atom::ParaLeft << "Inherited by ";
            appendSortedQmlNames(text, qcn, subs);
            text << Atom::ParaRight;
            generateText(text, qcn, marker);
        }
    }
}

/*!
  Extract sections of markup text surrounded by \e qmltext
  and \e endqmltext and output them.
 */
bool Generator::generateQmlText(const Text &text, const Node *relative, CodeMarker *marker,
                                const QString & /* qmlName */)
{
    const Atom *atom = text.firstAtom();
    bool result = false;

    if (atom != nullptr) {
        initializeTextOutput();
        while (atom) {
            if (atom->type() != Atom::QmlText)
                atom = atom->next();
            else {
                atom = atom->next();
                while (atom && (atom->type() != Atom::EndQmlText)) {
                    int n = 1 + generateAtom(atom, relative, marker);
                    while (n-- > 0)
                        atom = atom->next();
                }
            }
        }
        result = true;
    }
    return result;
}

void Generator::generateReimplementsClause(const FunctionNode *fn, CodeMarker *marker)
{
    if (!fn->overridesThis().isEmpty()) {
        if (fn->parent()->isClassNode()) {
            ClassNode *cn = static_cast<ClassNode *>(fn->parent());
            const FunctionNode *overrides = cn->findOverriddenFunction(fn);
            if (overrides && !overrides->isPrivate() && !overrides->parent()->isPrivate()) {
                if (overrides->hasDoc()) {
                    Text text;
                    text << Atom::ParaLeft << "Reimplements: ";
                    QString fullName =
                            overrides->parent()->name() + "::" + overrides->signature(false, true);
                    appendFullName(text, overrides->parent(), fullName, overrides);
                    text << "." << Atom::ParaRight;
                    generateText(text, fn, marker);
                } else {
                    fn->doc().location().warning(
                            tr("Illegal \\reimp; no documented virtual function for %1")
                                    .arg(overrides->plainSignature()));
                }
                return;
            }
            const PropertyNode *sameName = cn->findOverriddenProperty(fn);
            if (sameName && sameName->hasDoc()) {
                Text text;
                text << Atom::ParaLeft << "Reimplements an access function for property: ";
                QString fullName = sameName->parent()->name() + "::" + sameName->name();
                appendFullName(text, sameName->parent(), fullName, sameName);
                text << "." << Atom::ParaRight;
                generateText(text, fn, marker);
            }
        }
    }
}

QString Generator::formatSince(const Node *node)
{
    QStringList since = node->since().split(QLatin1Char(' '));

    // If there is only one argument, assume it is the Qt version number.
    if (since.count() == 1)
        return "Qt " + since[0];

    // Otherwise, use the original <project> <version> string.
    return node->since();
}

void Generator::generateSince(const Node *node, CodeMarker *marker)
{
    if (!node->since().isEmpty()) {
        Text text;
        text << Atom::ParaLeft << "This " << typeString(node) << " was introduced ";
        if (node->isEnumType())
            text << "or modified ";
        text << "in " << formatSince(node) << "." << Atom::ParaRight;
        generateText(text, node, marker);
    }
}

void Generator::generateStatus(const Node *node, CodeMarker *marker)
{
    Text text;

    switch (node->status()) {
    case Node::Active:
        // Do nothing.
        break;
    case Node::Preliminary:
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD) << "This "
             << typeString(node) << " is under development and is subject to change."
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD) << Atom::ParaRight;
        break;
    case Node::Deprecated:
        text << Atom::ParaLeft;
        if (node->isAggregate())
            text << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD);
        text << "This " << typeString(node) << " is deprecated.";
        if (node->isAggregate())
            text << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);
        text << Atom::ParaRight;
        break;
    case Node::Obsolete:
        text << Atom::ParaLeft;
        if (node->isAggregate())
            text << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD);
        text << "This " << typeString(node) << " is obsolete.";
        if (node->isAggregate())
            text << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);
        text << " It is provided to keep old source code working. "
             << "We strongly advise against "
             << "using it in new code." << Atom::ParaRight;
        break;
    case Node::Internal:
    default:
        break;
    }
    generateText(text, node, marker);
}

/*!
  Generates an addendum note of type \a type for \a node, using \a marker
  as the code marker.
*/
void Generator::generateAddendum(const Node *node, Addendum type, CodeMarker *marker,
                                 bool generateNote)
{
    Q_ASSERT(node && !node->name().isEmpty());
    Text text;
    text << Atom::ParaLeft;

    if (generateNote) {
        text  << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
              << "Note: " << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);
    }

    switch (type) {
    case Invokable:
        text << "This function can be invoked via the meta-object system and from QML. See "
             << Atom(Atom::Link, "Q_INVOKABLE")
             << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << "Q_INVOKABLE"
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << ".";
        break;
    case PrivateSignal:
        text << "This is a private signal. It can be used in signal connections "
                "but cannot be emitted by the user.";
        break;
    case QmlSignalHandler:
    {
        QString handler(node->name());
        handler[0] = handler[0].toTitleCase();
        handler.prepend(QLatin1String("on"));
        text << "The corresponding handler is "
             << Atom(Atom::FormattingLeft, ATOM_FORMATTING_TELETYPE) << handler
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_TELETYPE) << ".";
        break;
    }
    case AssociatedProperties:
    {
        if (!node->isFunction())
            return;
        const FunctionNode *fn = static_cast<const FunctionNode *>(node);
        NodeList nodes = fn->associatedProperties();
        if (nodes.isEmpty())
            return;
        std::sort(nodes.begin(), nodes.end(), Node::nodeNameLessThan);
        for (const auto *n : qAsConst(nodes)) {
            QString msg;
            const PropertyNode *pn = static_cast<const PropertyNode *>(n);
            switch (pn->role(fn)) {
            case PropertyNode::Getter:
                msg = QStringLiteral("Getter function");
                break;
            case PropertyNode::Setter:
                msg = QStringLiteral("Setter function");
                break;
            case PropertyNode::Resetter:
                msg = QStringLiteral("Resetter function");
                break;
            case PropertyNode::Notifier:
                msg = QStringLiteral("Notifier signal");
                break;
            default:
                continue;
            }
            text << msg << " for property " << Atom(Atom::Link, pn->name())
             << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << pn->name()
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << ". ";
        }
        break;
    }
    case TypeAlias:
    {
        if (!node->isTypeAlias())
            return;
        const auto *ta = static_cast<const TypeAliasNode *>(node);
        text << "This is a type alias for ";
        if (ta->aliasedNode() && ta->aliasedNode()->isInAPI()) {
            text << Atom(Atom::LinkNode, CodeMarker::stringForNode(ta->aliasedNode()))
                 << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
                 << Atom(Atom::String, ta->aliasedNode()->plainFullName(ta->parent()))
                 << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << ".";
        } else {
            text << Atom(Atom::String, ta->aliasedType()) << ".";
        }
        break;
    }
    default:
        return;
    }

    text << Atom::ParaRight;
    generateText(text, node, marker);
}

/*!
  Generate the documentation for \a relative. i.e. \a relative
  is the node that represents the entity where a qdoc comment
  was found, and \a text represents the qdoc comment.
 */
bool Generator::generateText(const Text &text, const Node *relative, CodeMarker *marker)
{
    bool result = false;
    if (text.firstAtom() != nullptr) {
        int numAtoms = 0;
        initializeTextOutput();
        generateAtomList(text.firstAtom(), relative, marker, true, numAtoms);
        result = true;
    }
    return result;
}

/*
  The node is an aggregate, typically a class node, which has
  a threadsafeness level. This function checks all the children
  of the node to see if they are exceptions to the node's
  threadsafeness. If there are any exceptions, the exceptions
  are added to the appropriate set (reentrant, threadsafe, and
  nonreentrant, and true is returned. If there are no exceptions,
  the three node lists remain empty and false is returned.
 */
bool Generator::hasExceptions(const Node *node, NodeList &reentrant, NodeList &threadsafe,
                              NodeList &nonreentrant)
{
    bool result = false;
    Node::ThreadSafeness ts = node->threadSafeness();
    const NodeList &children = static_cast<const Aggregate *>(node)->childNodes();
    for (auto child : children) {
        if (!child->isObsolete()) {
            switch (child->threadSafeness()) {
            case Node::Reentrant:
                reentrant.append(child);
                if (ts == Node::ThreadSafe)
                    result = true;
                break;
            case Node::ThreadSafe:
                threadsafe.append(child);
                if (ts == Node::Reentrant)
                    result = true;
                break;
            case Node::NonReentrant:
                nonreentrant.append(child);
                result = true;
                break;
            default:
                break;
            }
        }
    }
    return result;
}

static void startNote(Text &text)
{
    text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
         << "Note:" << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD) << " ";
}

/*!
  Generates text that explains how threadsafe and/or reentrant
  \a node is.
 */
void Generator::generateThreadSafeness(const Node *node, CodeMarker *marker)
{
    Text text, rlink, tlink;
    NodeList reentrant;
    NodeList threadsafe;
    NodeList nonreentrant;
    Node::ThreadSafeness ts = node->threadSafeness();
    bool exceptions = false;

    rlink << Atom(Atom::Link, "reentrant") << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
          << "reentrant" << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);

    tlink << Atom(Atom::Link, "thread-safe") << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
          << "thread-safe" << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);

    switch (ts) {
    case Node::UnspecifiedSafeness:
        break;
    case Node::NonReentrant:
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
             << "Warning:" << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD) << " This "
             << typeString(node) << " is not " << rlink << "." << Atom::ParaRight;
        break;
    case Node::Reentrant:
    case Node::ThreadSafe:
        startNote(text);
        if (node->isAggregate()) {
            exceptions = hasExceptions(node, reentrant, threadsafe, nonreentrant);
            text << "All functions in this " << typeString(node) << " are ";
            if (ts == Node::ThreadSafe)
                text << tlink;
            else
                text << rlink;

            if (!exceptions || (ts == Node::Reentrant && !threadsafe.isEmpty()))
                text << ".";
            else
                text << " with the following exceptions:";
        } else {
            text << "This " << typeString(node) << " is ";
            if (ts == Node::ThreadSafe)
                text << tlink;
            else
                text << rlink;
            text << ".";
        }
        text << Atom::ParaRight;
        break;
    default:
        break;
    }
    generateText(text, node, marker);

    if (exceptions) {
        text.clear();
        if (ts == Node::Reentrant) {
            if (!nonreentrant.isEmpty()) {
                startNote(text);
                text << "These functions are not " << rlink << ":" << Atom::ParaRight;
                signatureList(nonreentrant, node, marker);
            }
            if (!threadsafe.isEmpty()) {
                text.clear();
                startNote(text);
                text << "These functions are also " << tlink << ":" << Atom::ParaRight;
                generateText(text, node, marker);
                signatureList(threadsafe, node, marker);
            }
        } else { // thread-safe
            if (!reentrant.isEmpty()) {
                startNote(text);
                text << "These functions are only " << rlink << ":" << Atom::ParaRight;
                signatureList(reentrant, node, marker);
            }
            if (!nonreentrant.isEmpty()) {
                text.clear();
                startNote(text);
                text << "These functions are not " << rlink << ":" << Atom::ParaRight;
                signatureList(nonreentrant, node, marker);
            }
        }
    }
}

/*!
  Returns the string containing an example code of the input node,
  if it is an overloaded signal. Otherwise, returns an empty string.
 */
QString Generator::getOverloadedSignalCode(const Node *node)
{
    if (!node->isFunction())
        return QString();
    const auto func = static_cast<const FunctionNode *>(node);
    if (!func->isSignal() || !func->hasOverloads())
        return QString();

    // Compute a friendly name for the object of that instance.
    // e.g:  "QAbstractSocket" -> "abstractSocket"
    QString objectName = node->parent()->name();
    if (objectName.size() >= 2) {
        if (objectName[0] == 'Q')
            objectName = objectName.mid(1);
        objectName[0] = objectName[0].toLower();
    }

    // We have an overloaded signal, show an example. Note, for const
    // overloaded signals, one should use Q{Const,NonConst}Overload, but
    // it is very unlikely that we will ever have public API overloading
    // signals by const.
    QString code = "connect(" + objectName + ", QOverload<";
    code += func->parameters().generateTypeList();
    code += ">::of(&" + func->parent()->name() + "::" + func->name() + "),\n    [=](";
    code += func->parameters().generateTypeAndNameList();
    code += "){ /* ... */ });";

    return code;
}

/*!
    If the node is an overloaded signal, add a node with an example on how to connect to it
 */
void Generator::generateOverloadedSignal(const Node *node, CodeMarker *marker)
{
    QString code = getOverloadedSignalCode(node);
    if (code.isEmpty())
        return;

    Text text;
    text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
         << "Note:" << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD) << " Signal "
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_ITALIC) << node->name()
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_ITALIC)
         << " is overloaded in this class. "
            "To connect to this signal by using the function pointer syntax, Qt "
            "provides a convenient helper for obtaining the function pointer as "
            "shown in this example:"
         << Atom(Atom::Code, marker->markedUpCode(code, node, node->location()));

    generateText(text, node, marker);
}

/*!
  Traverses the database recursively to generate all the documentation.
 */
void Generator::generateDocs()
{
    currentGenerator_ = this;
    generateDocumentation(qdb_->primaryTreeRoot());
}

Generator *Generator::generatorForFormat(const QString &format)
{
    for (const auto &generator : qAsConst(generators)) {
        if (generator->format() == format)
            return generator;
    }
    return nullptr;
}

/*!
  Looks up the tag \a tag in the map of metadata values for the
  current topic in \a inner. If a value for the tag is found,
  the value is returned.

  \note If \a tag is found in the metadata map, it is erased.
  i.e. Once you call this function for a particular \a tag,
  you consume \a tag.
 */
QString Generator::getMetadataElement(const Aggregate *inner, const QString &tag)
{
    QString s;
    QStringMultiMap &metaTagMap = const_cast<QStringMultiMap &>(inner->doc().metaTagMap());
    for (auto it = metaTagMap.find(tag); it != metaTagMap.end();) {
        s = it.value();
        metaTagMap.erase(it);
    }
    return s;
}

/*!
  Looks up the tag \a t in the map of metadata values for the
  current topic in \a inner. If values for the tag are found,
  they are returned in a string list.

  \note If \a t is found in the metadata map, all the pairs
  having the key \a t are erased. i.e. Once you call this
  function for a particular \a t, you consume \a t.
 */
QStringList Generator::getMetadataElements(const Aggregate *inner, const QString &t)
{
    QStringList s;
    QStringMultiMap &metaTagMap = const_cast<QStringMultiMap &>(inner->doc().metaTagMap());
    s = metaTagMap.values(t);
    if (!s.isEmpty())
        metaTagMap.remove(t);
    return s;
}

/*!
  Returns a relative path name for an image.
 */
QString Generator::imageFileName(const Node *relative, const QString &fileBase)
{
    QString userFriendlyFilePath;
    QString filePath = Config::findFile(relative->doc().location(), imageFiles, imageDirs, fileBase,
                                        imgFileExts[format()], &userFriendlyFilePath);

    if (filePath.isEmpty())
        return QString();

    QString path = Config::copyFile(relative->doc().location(), filePath, userFriendlyFilePath,
                                    outputDir() + QLatin1String("/images"));
    int images_slash = path.lastIndexOf("images/");
    QString relImagePath;
    if (images_slash != -1)
        relImagePath = path.mid(images_slash);
    return relImagePath;
}

QString Generator::indent(int level, const QString &markedCode)
{
    if (level == 0)
        return markedCode;

    QString t;
    int column = 0;

    int i = 0;
    while (i < markedCode.length()) {
        if (markedCode.at(i) == QLatin1Char('\n')) {
            column = 0;
        } else {
            if (column == 0) {
                for (int j = 0; j < level; j++)
                    t += QLatin1Char(' ');
            }
            column++;
        }
        t += markedCode.at(i++);
    }
    return t;
}

void Generator::initialize()
{
    Config &config = Config::instance();
    outputFormats = config.getOutputFormats();
    redirectDocumentationToDevNull_ = config.getBool(CONFIG_REDIRECTDOCUMENTATIONTODEVNULL);

    imageFiles = config.getCanonicalPathList(CONFIG_IMAGES);
    imageDirs = config.getCanonicalPathList(CONFIG_IMAGEDIRS);
    scriptFiles = config.getCanonicalPathList(CONFIG_SCRIPTS);
    scriptDirs = config.getCanonicalPathList(CONFIG_SCRIPTDIRS);
    styleFiles = config.getCanonicalPathList(CONFIG_STYLES);
    styleDirs = config.getCanonicalPathList(CONFIG_STYLEDIRS);
    exampleDirs = config.getCanonicalPathList(CONFIG_EXAMPLEDIRS);
    exampleImgExts = config.getStringList(CONFIG_EXAMPLES + Config::dot + CONFIG_IMAGEEXTENSIONS);

    QString imagesDotFileExtensions = CONFIG_IMAGES + Config::dot + CONFIG_FILEEXTENSIONS;
    for (const auto &ext : config.subVars(imagesDotFileExtensions))
        imgFileExts[ext] = config.getStringList(imagesDotFileExtensions + Config::dot + ext);

    for (auto &g : generators) {
        if (outputFormats.contains(g->format())) {
            currentGenerator_ = g;
            g->initializeGenerator();
        }
    }

    for (const auto &n : config.subVars(CONFIG_FORMATTING)) {
        QString formattingDotName = CONFIG_FORMATTING + Config::dot + n;
        for (const auto &f : config.subVars(formattingDotName)) {
            QString def = config.getString(formattingDotName + Config::dot + f);
            if (!def.isEmpty()) {
                int numParams = Config::numParams(def);
                int numOccs = def.count("\1");
                if (numParams != 1) {
                    config.lastLocation().warning(tr("Formatting '%1' must "
                                                     "have exactly one "
                                                     "parameter (found %2)")
                                                          .arg(n)
                                                          .arg(numParams));
                } else if (numOccs > 1) {
                    config.lastLocation().fatal(tr("Formatting '%1' must "
                                                   "contain exactly one "
                                                   "occurrence of '\\1' "
                                                   "(found %2)")
                                                        .arg(n)
                                                        .arg(numOccs));
                } else {
                    int paramPos = def.indexOf("\1");
                    fmtLeftMaps[f].insert(n, def.left(paramPos));
                    fmtRightMaps[f].insert(n, def.mid(paramPos + 1));
                }
            }
        }
    }

    project_ = config.getString(CONFIG_PROJECT);
    outDir_ = config.getOutputDir();
    outSubdir_ = outDir_.mid(outDir_.lastIndexOf('/') + 1);

    outputPrefixes.clear();
    QStringList items = config.getStringList(CONFIG_OUTPUTPREFIXES);
    if (!items.isEmpty()) {
        for (const auto &prefix : items)
            outputPrefixes[prefix] = config.getString(CONFIG_OUTPUTPREFIXES + Config::dot + prefix);
    } else {
        outputPrefixes[QLatin1String("QML")] = QLatin1String("qml-");
        outputPrefixes[QLatin1String("JS")] = QLatin1String("js-");
    }

    outputSuffixes.clear();
    for (const auto &suffix : config.getStringList(CONFIG_OUTPUTSUFFIXES))
        outputSuffixes[suffix] = config.getString(CONFIG_OUTPUTSUFFIXES + Config::dot + suffix);

    noLinkErrors_ = config.getBool(CONFIG_NOLINKERRORS);
    autolinkErrors_ = config.getBool(CONFIG_AUTOLINKERRORS);
}

/*!
  Creates template-specific subdirs (e.g. /styles and /scripts for HTML)
  and copies the files to them.
  */
void Generator::copyTemplateFiles(const QString &configVar, const QString &subDir)
{
    Config &config = Config::instance();
    QStringList files = config.getCanonicalPathList(configVar, true);
    if (!files.isEmpty()) {
        QDir dirInfo;
        QString templateDir = outDir_ + QLatin1Char('/') + subDir;
        if (!dirInfo.exists(templateDir) && !dirInfo.mkdir(templateDir)) {
            config.lastLocation().fatal(
                    tr("Cannot create %1 directory '%2'").arg(subDir, templateDir));
        } else {
            for (const auto &file : files) {
                if (!file.isEmpty())
                    Config::copyFile(config.lastLocation(), file, file, templateDir);
            }
        }
    }
}

/*!
    Reads format-specific variables from config, sets output
    (sub)directories, creates them on the filesystem and copies the
    template-specific files.
 */
void Generator::initializeFormat()
{
    Config &config = Config::instance();
    outFileNames_.clear();
    useOutputSubdirs_ = true;
    if (config.getBool(format() + Config::dot + "nosubdirs"))
        resetUseOutputSubdirs();

    if (outputFormats.isEmpty())
        return;

    outDir_ = config.getOutputDir(format());
    if (outDir_.isEmpty()) {
        config.lastLocation().fatal(tr("No output directory specified in "
                                       "configuration file or on the command line"));
    } else {
        outSubdir_ = outDir_.mid(outDir_.lastIndexOf('/') + 1);
    }

    QDir dirInfo;
    if (dirInfo.exists(outDir_)) {
        if (!config.generating() && Generator::useOutputSubdirs()) {
            if (!Config::removeDirContents(outDir_))
                config.lastLocation().error(tr("Cannot empty output directory '%1'").arg(outDir_));
        }
    } else if (!dirInfo.mkpath(outDir_)) {
        config.lastLocation().fatal(tr("Cannot create output directory '%1'").arg(outDir_));
    }

    // Output directory exists, which is enough for prepare phase.
    if (config.preparing())
        return;

    if (!dirInfo.exists(outDir_ + "/images") && !dirInfo.mkdir(outDir_ + "/images"))
        config.lastLocation().fatal(
                tr("Cannot create images directory '%1'").arg(outDir_ + "/images"));

    copyTemplateFiles(format() + Config::dot + CONFIG_STYLESHEETS, "style");
    copyTemplateFiles(format() + Config::dot + CONFIG_SCRIPTS, "scripts");
    copyTemplateFiles(format() + Config::dot + CONFIG_EXTRAIMAGES, "images");

    // Use a format-specific .quotinginformation if defined, otherwise a global value
    if (config.subVars(format()).contains(CONFIG_QUOTINGINFORMATION))
        quoting_ = config.getBool(format() + Config::dot + CONFIG_QUOTINGINFORMATION);
    else
        quoting_ = config.getBool(CONFIG_QUOTINGINFORMATION);
}

/*!
  Appends each directory path in \a moreImageDirs to the
  list of image directories.
 */
void Generator::augmentImageDirs(QSet<QString> &moreImageDirs)
{
    if (moreImageDirs.isEmpty())
        return;
    for (const auto &it : moreImageDirs)
        imageDirs.append(it);
}

/*!
  Sets the generator's pointer to the Config instance.
 */
void Generator::initializeGenerator()
{
    showInternal_ = Config::instance().getBool(CONFIG_SHOWINTERNAL);
    singleExec_ = Config::instance().getBool(CONFIG_SINGLEEXEC);
}

bool Generator::matchAhead(const Atom *atom, Atom::AtomType expectedAtomType)
{
    return atom->next() && atom->next()->type() == expectedAtomType;
}

/*!
  Used for writing to the current output stream. Returns a
  reference to the current output stream, which is then used
  with the \c {<<} operator for writing.
 */
QTextStream &Generator::out()
{
    return *outStreamStack.top();
}

QString Generator::outFileName()
{
    return QFileInfo(static_cast<QFile *>(out().device())->fileName()).fileName();
}

QString Generator::outputPrefix(const Node *node)
{
    // Prefix is applied to QML and JS types
    if (node->isQmlType() || node->isQmlBasicType())
        return outputPrefixes[QLatin1String("QML")];
    if (node->isJsType() || node->isJsBasicType())
        return outputPrefixes[QLatin1String("JS")];
    return QString();
}

QString Generator::outputSuffix(const Node *node)
{
    // Suffix is applied to QML and JS types, as
    // well as module pages.
    if (node->isQmlModule() || node->isQmlType() || node->isQmlBasicType())
        return outputSuffixes[QLatin1String("QML")];
    if (node->isJsModule() || node->isJsType() || node->isJsBasicType())
        return outputSuffixes[QLatin1String("JS")];
    return QString();
}

bool Generator::parseArg(const QString &src, const QString &tag, int *pos, int n,
                         QStringRef *contents, QStringRef *par1, bool debug)
{
#define SKIP_CHAR(c)                                                                               \
    if (debug)                                                                                     \
        qDebug() << "looking for " << c << " at " << QString(src.data() + i, n - i);               \
    if (i >= n || src[i] != c) {                                                                   \
        if (debug)                                                                                 \
            qDebug() << " char '" << c << "' not found";                                           \
        return false;                                                                              \
    }                                                                                              \
    ++i;

#define SKIP_SPACE                                                                                 \
    while (i < n && src[i] == ' ')                                                                 \
        ++i;

    int i = *pos;
    int j = i;

    // assume "<@" has been parsed outside
    // SKIP_CHAR('<');
    // SKIP_CHAR('@');

    if (tag != QStringRef(&src, i, tag.length())) {
        return false;
    }

    if (debug)
        qDebug() << "haystack:" << src << "needle:" << tag << "i:" << i;

    // skip tag
    i += tag.length();

    // parse stuff like:  linkTag("(<@link node=\"([^\"]+)\">).*(</@link>)");
    if (par1) {
        SKIP_SPACE;
        // read parameter name
        j = i;
        while (i < n && src[i].isLetter())
            ++i;
        if (src[i] == '=') {
            if (debug)
                qDebug() << "read parameter" << QString(src.data() + j, i - j);
            SKIP_CHAR('=');
            SKIP_CHAR('"');
            // skip parameter name
            j = i;
            while (i < n && src[i] != '"')
                ++i;
            *par1 = QStringRef(&src, j, i - j);
            SKIP_CHAR('"');
            SKIP_SPACE;
        } else {
            if (debug)
                qDebug() << "no optional parameter found";
        }
    }
    SKIP_SPACE;
    SKIP_CHAR('>');

    // find contents up to closing "</@tag>
    j = i;
    for (; true; ++i) {
        if (i + 4 + tag.length() > n)
            return false;
        if (src[i] != '<')
            continue;
        if (src[i + 1] != '/')
            continue;
        if (src[i + 2] != '@')
            continue;
        if (tag != QStringRef(&src, i + 3, tag.length()))
            continue;
        if (src[i + 3 + tag.length()] != '>')
            continue;
        break;
    }

    *contents = QStringRef(&src, j, i - j);

    i += tag.length() + 4;

    *pos = i;
    if (debug)
        qDebug() << " tag " << tag << " found: pos now: " << i;
    return true;
#undef SKIP_CHAR
}

QString Generator::plainCode(const QString &markedCode)
{
    QString t = markedCode;
    t.replace(tag, QString());
    t.replace(quot, QLatin1String("\""));
    t.replace(gt, QLatin1String(">"));
    t.replace(lt, QLatin1String("<"));
    t.replace(amp, QLatin1String("&"));
    return t;
}

void Generator::setImageFileExtensions(const QStringList &extensions)
{
    imgFileExts[format()] = extensions;
}

void Generator::singularPlural(Text &text, const NodeList &nodes)
{
    if (nodes.count() == 1)
        text << " is";
    else
        text << " are";
}

int Generator::skipAtoms(const Atom *atom, Atom::AtomType type) const
{
    int skipAhead = 0;
    atom = atom->next();
    while (atom && atom->type() != type) {
        skipAhead++;
        atom = atom->next();
    }
    return skipAhead;
}

/*!
  Resets the variables used during text output.
 */
void Generator::initializeTextOutput()
{
    inLink_ = false;
    inContents_ = false;
    inSectionHeading_ = false;
    inTableHeader_ = false;
    numTableRows_ = 0;
    threeColumnEnumValueTable_ = true;
    link_.clear();
    sectionNumber_.clear();
}

void Generator::supplementAlsoList(const Node *node, QVector<Text> &alsoList)
{
    if (node->isFunction() && !node->isMacro()) {
        const auto fn = static_cast<const FunctionNode *>(node);
        if (fn->overloadNumber() == 0) {
            QString alternateName;
            const FunctionNode *alternateFunc = nullptr;

            if (fn->name().startsWith("set") && fn->name().size() >= 4) {
                alternateName = fn->name()[3].toLower();
                alternateName += fn->name().mid(4);
                alternateFunc = fn->parent()->findFunctionChild(alternateName, QString());

                if (!alternateFunc) {
                    alternateName = "is" + fn->name().mid(3);
                    alternateFunc = fn->parent()->findFunctionChild(alternateName, QString());
                    if (!alternateFunc) {
                        alternateName = "has" + fn->name().mid(3);
                        alternateFunc = fn->parent()->findFunctionChild(alternateName, QString());
                    }
                }
            } else if (!fn->name().isEmpty()) {
                alternateName = "set";
                alternateName += fn->name()[0].toUpper();
                alternateName += fn->name().mid(1);
                alternateFunc = fn->parent()->findFunctionChild(alternateName, QString());
            }

            if (alternateFunc && alternateFunc->access() != Node::Private) {
                int i;
                for (i = 0; i < alsoList.size(); ++i) {
                    if (alsoList.at(i).toString().contains(alternateName))
                        break;
                }

                if (i == alsoList.size()) {
                    alternateName += "()";

                    Text also;
                    also << Atom(Atom::Link, alternateName)
                         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << alternateName
                         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
                    alsoList.prepend(also);
                }
            }
        }
    }
}

void Generator::terminate()
{
    for (const auto &generator : qAsConst(generators)) {
        if (outputFormats.contains(generator->format()))
            generator->terminateGenerator();
    }

    fmtLeftMaps.clear();
    fmtRightMaps.clear();
    imgFileExts.clear();
    imageFiles.clear();
    imageDirs.clear();
    outDir_.clear();
}

void Generator::terminateGenerator() {}

/*!
  Trims trailing whitespace off the \a string and returns
  the trimmed string.
 */
QString Generator::trimmedTrailing(const QString &string, const QString &prefix,
                                   const QString &suffix)
{
    QString trimmed = string;
    while (trimmed.length() > 0 && trimmed[trimmed.length() - 1].isSpace())
        trimmed.truncate(trimmed.length() - 1);

    trimmed.append(suffix);
    trimmed.prepend(prefix);
    return trimmed;
}

QString Generator::typeString(const Node *node)
{
    switch (node->nodeType()) {
    case Node::Namespace:
        return "namespace";
    case Node::Class:
        return "class";
    case Node::Struct:
        return "struct";
    case Node::Union:
        return "union";
    case Node::QmlType:
    case Node::QmlBasicType:
    case Node::JsBasicType:
        return "type";
    case Node::Page:
        return "documentation";
    case Node::Enum:
        return "enum";
    case Node::Typedef:
        return "typedef";
    case Node::TypeAlias:
        return "alias";
    case Node::Function: {
        const auto fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::JsSignal:
        case FunctionNode::QmlSignal:
            return "signal";
        case FunctionNode::JsSignalHandler:
        case FunctionNode::QmlSignalHandler:
            return "signal handler";
        case FunctionNode::JsMethod:
        case FunctionNode::QmlMethod:
            return "method";
        default:
            break;
        }
        return "function";
    }
    case Node::Property:
    case Node::QmlProperty:
        return "property";
    case Node::Module:
    case Node::JsModule:
    case Node::QmlModule:
        return "module";
    case Node::SharedComment: {
        const auto &collective = static_cast<const SharedCommentNode *>(node)->collective();
        return collective.first()->nodeTypeString();
    }
    default:
        return "documentation";
    }
}

void Generator::unknownAtom(const Atom *atom)
{
    Location::internalError(
            tr("unknown atom type '%1' in %2 generator").arg(atom->typeString()).arg(format()));
}

QT_END_NAMESPACE
