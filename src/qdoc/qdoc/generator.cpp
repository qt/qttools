// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "generator.h"

#include "access.h"
#include "aggregate.h"
#include "classnode.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "doc.h"
#include "editdistance.h"
#include "enumnode.h"
#include "examplenode.h"
#include "functionnode.h"
#include "node.h"
#include "openedlist.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "quoter.h"
#include "sharedcommentnode.h"
#include "tokenizer.h"
#include "typedefnode.h"
#include "utilities.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qregularexpression.h>

#ifndef QT_BOOTSTRAPPED
#    include "QtCore/qurl.h"
#endif

#include <string>

using namespace std::literals::string_literals;

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Generator *Generator::s_currentGenerator;
QMap<QString, QMap<QString, QString>> Generator::s_fmtLeftMaps;
QMap<QString, QMap<QString, QString>> Generator::s_fmtRightMaps;
QList<Generator *> Generator::s_generators;
QString Generator::s_outDir;
QString Generator::s_outSubdir;
QStringList Generator::s_outFileNames;
QSet<QString> Generator::s_outputFormats;
QHash<QString, QString> Generator::s_outputPrefixes;
QHash<QString, QString> Generator::s_outputSuffixes;
QString Generator::s_project;
bool Generator::s_noLinkErrors = false;
bool Generator::s_autolinkErrors = false;
bool Generator::s_redirectDocumentationToDevNull = false;
bool Generator::s_useOutputSubdirs = true;
QmlTypeNode *Generator::s_qmlTypeContext = nullptr;

static QRegularExpression tag("</?@[^>]*>");
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
Generator::Generator(FileResolver& file_resolver)
    : file_resolver{file_resolver}
{
    m_qdb = QDocDatabase::qdocDB();
    s_generators.prepend(this);
}

/*!
  Destroys the generator after removing it from the list of
  output generators.
 */
Generator::~Generator()
{
    s_generators.removeAll(this);
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

/*!
  Append the signature for the function named in \a node to
  \a text, so that is a link to the documentation for that
  function.
 */
void Generator::appendSignature(Text &text, const Node *node)
{
    text << Atom(Atom::LinkNode, CodeMarker::stringForNode(node))
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
         << Atom(Atom::String, node->signature(Node::SignaturePlain))
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

int Generator::appendSortedNames(Text &text, const ClassNode *cn, const QList<RelatedClass> &rc)
{
    QMap<QString, Text> classMap;
    for (const auto &relatedClass : rc) {
        ClassNode *rcn = relatedClass.m_node;
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
        text << Utilities::comma(index++, classNames.size());
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
        text << Utilities::comma(index++, names.size());
    }
    return index;
}

/*!
  Creates the file named \a fileName in the output directory
  and returns a QFile pointing to this file. In particular,
  this method deals with errors when opening the file:
  the returned QFile is always valid and can be written to.

  \sa beginSubPage()
 */
QFile *Generator::openSubPageFile(const Node *node, const QString &fileName)
{
    QString path = outputDir() + QLatin1Char('/');
    if (Generator::useOutputSubdirs() && !node->outputSubdirectory().isEmpty()
        && !outputDir().endsWith(node->outputSubdirectory())) {
        path += node->outputSubdirectory() + QLatin1Char('/');
    }
    path += fileName;

    auto outPath = s_redirectDocumentationToDevNull ? QStringLiteral("/dev/null") : path;
    auto outFile = new QFile(outPath);

    if (!s_redirectDocumentationToDevNull && outFile->exists())
        qCDebug(lcQdoc) << "Output file already exists; overwriting" << qPrintable(outFile->fileName());

    if (!outFile->open(QFile::WriteOnly | QFile::Text)) {
        node->location().fatal(
                QStringLiteral("Cannot open output file '%1'").arg(outFile->fileName()));
    }

    qCDebug(lcQdoc, "Writing: %s", qPrintable(path));
    s_outFileNames << fileName;
    return outFile;
}

/*!
  Creates the file named \a fileName in the output directory.
  Attaches a QTextStream to the created file, which is written
  to all over the place using out().
 */
void Generator::beginSubPage(const Node *node, const QString &fileName)
{
    QFile *outFile = openSubPageFile(node, fileName);
    auto *out = new QTextStream(outFile);
    outStreamStack.push(out);
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
            base.truncate(base.size() - 5);

        if (node->isQmlModule())
            base.append("-qmlmodule");
        else if (node->isModule())
            base.append("-module");
        // Why not add "-group" for group pages?
    } else if (node->isTextPageNode()) {
        base = node->name();
        if (base.endsWith(".html"))
            base.truncate(base.size() - 5);

        if (node->isExample()) {
            base.prepend(s_project.toLower() + QLatin1Char('-'));
            base.append(QLatin1String("-example"));
        }
    } else if (node->isQmlType()) {
        base = node->name();
        /*
          To avoid file name conflicts in the html directory,
          we prepend a prefix (by default, "qml-") and an optional suffix
          to the file name. The suffix, if one exists, is appended to the
          module name.

          For historical reasons, skip the module name qualifier for QML value types
          in order to avoid excess redirects in the online docs. TODO: re-assess
        */
        if (!node->logicalModuleName().isEmpty() && !node->isQmlBasicType()
            && (!node->logicalModule()->isInternal() || m_showInternal))
            base.prepend(node->logicalModuleName() + outputSuffix(node) + QLatin1Char('-'));

        base.prepend(outputPrefix(node));
    } else if (node->isProxyNode()) {
        base.append("%1-%2-proxy"_L1.arg(node->name(), node->tree()->physicalModuleName()));
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
            const auto *ns = static_cast<const NamespaceNode *>(node);
            if (!ns->isDocumentedHere()) {
                base.append(QLatin1String("-sub-"));
                base.append(ns->tree()->camelCaseModuleName());
            }
        }
    }

    QString canonicalName{ Utilities::asAsciiPrintable(base) };
    Node *n = const_cast<Node *>(node);
    n->setFileNameBase(canonicalName);
    return canonicalName;
}

/*!
  Constructs an href link from an example file name, which
  is a \a path to the example file. If \a fileExt is empty
  (default value), retrieve the file extension from
  the generator.
 */
QString Generator::linkForExampleFile(const QString &path, const QString &fileExt)
{
    QString link{path};
    link.prepend(s_project.toLower() + QLatin1Char('-'));

    QString canonicalName{ Utilities::asAsciiPrintable(link) };
    canonicalName.append(QLatin1Char('.'));
    canonicalName.append(fileExt.isEmpty() ? fileExtension() : fileExt);
    return canonicalName;
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
    return name + (extension.isNull() ? fileExtension() : extension);
}

/*!
  Clean the given \a ref to be used as an HTML anchor or an \c xml:id.
  If \a xmlCompliant is set to \c true, a stricter process is used, as XML
  is more rigorous in what it accepts. Otherwise, if \a xmlCompliant is set to
  \c false, the basic HTML transformations are applied.

  More specifically, only XML NCNames are allowed
  (https://www.w3.org/TR/REC-xml-names/#NT-NCName).
 */
QString Generator::cleanRef(const QString &ref, bool xmlCompliant)
{
    // XML-compliance is ensured in two ways:
    // - no digit (0-9) at the beginning of an ID (many IDs do not respect this property)
    // - no colon (:) anywhere in the ID (occurs very rarely)

    QString clean;

    if (ref.isEmpty())
        return clean;

    clean.reserve(ref.size() + 20);
    const QChar c = ref[0];
    const uint u = c.unicode();

    if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (!xmlCompliant && u >= '0' && u <= '9')) {
        clean += c;
    } else if (xmlCompliant && u >= '0' && u <= '9') {
        clean += QLatin1Char('A') + c;
    } else if (u == '~') {
        clean += "dtor.";
    } else if (u == '_') {
        clean += "underscore.";
    } else {
        clean += QLatin1Char('A');
    }

    for (int i = 1; i < ref.size(); i++) {
        const QChar c = ref[i];
        const uint u = c.unicode();
        if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || u == '-'
            || u == '_' || (xmlCompliant && u == ':') || u == '.') {
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
    return s_fmtLeftMaps[format()];
}

QMap<QString, QString> &Generator::formattingRightMap()
{
    return s_fmtRightMaps[format()];
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
    } else if (node->isQmlType()) {
        return fileBase(node) + QLatin1Char('.') + currentGenerator()->fileExtension();
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
        const auto *fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::QmlSignal:
            anchorRef = QLatin1Char('#') + node->name() + "-signal";
            break;
        case FunctionNode::QmlSignalHandler:
            anchorRef = QLatin1Char('#') + node->name() + "-signal-handler";
            break;
        case FunctionNode::QmlMethod:
            anchorRef = QLatin1Char('#') + node->name() + "-method";
            break;
        default:
            if (fn->isDtor())
                anchorRef = "#dtor." + fn->name().mid(1);
            else if (fn->hasOneAssociatedProperty() && fn->doc().isEmpty())
                return fullDocumentLocation(fn->associatedProperties()[0]);
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
    case Node::Typedef: {
        const auto *tdef = static_cast<const TypedefNode *>(node);
        if (tdef->associatedEnum())
            return fullDocumentLocation(tdef->associatedEnum());
    } Q_FALLTHROUGH();
    case Node::TypeAlias:
        anchorRef = QLatin1Char('#') + node->name() + "-typedef";
        break;
    case Node::Property:
        anchorRef = QLatin1Char('#') + node->name() + "-prop";
        break;
    case Node::SharedComment: {
        if (!node->isPropertyGroup())
            break;
    } Q_FALLTHROUGH();
    case Node::QmlProperty:
        if (node->isAttached())
            anchorRef = QLatin1Char('#') + node->name() + "-attached-prop";
        else
            anchorRef = QLatin1Char('#') + node->name() + "-prop";
        break;
    case Node::Variable:
        anchorRef = QLatin1Char('#') + node->name() + "-var";
        break;
    case Node::QmlType:
    case Node::Page:
    case Node::Group:
    case Node::HeaderFile:
    case Node::Module:
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
        if (node->isDeprecated())
            parentName.replace(QLatin1Char('.') + currentGenerator()->fileExtension(),
                               "-obsolete." + currentGenerator()->fileExtension());
    }

    return fdl + parentName.toLower() + anchorRef;
}

void Generator::generateAlsoList(const Node *node, CodeMarker *marker)
{
    QList<Text> alsoList = node->doc().alsoList();
    supplementAlsoList(node, alsoList);

    if (!alsoList.isEmpty()) {
        Text text;
        text << Atom::ParaLeft << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD) << "See also "
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);

        for (int i = 0; i < alsoList.size(); ++i)
            text << alsoList.at(i) << Utilities::separator(i, alsoList.size());

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
                    relative->location().warning(QStringLiteral("Output format %1 not handled %2")
                                                         .arg(format(), outFileName()));
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
                    node->location().warning(QStringLiteral("No documentation for '%1'")
                                                     .arg(node->plainSignature()));
            }
        } else if (!node->isWrapper() && !node->isMarkedReimp()) {
            // Don't require documentation of things defined in Q_GADGET
            if (node->name() != QLatin1String("QtGadgetHelper"))
                node->location().warning(
                        QStringLiteral("No documentation for '%1'").arg(node->plainSignature()));
        }
    } else if (!node->isSharingComment()) {
        // Reimplements clause and type alias info precede body text
        if (fn && !fn->overridesThis().isEmpty())
            generateReimplementsClause(fn, marker);
        else if (node->isProperty()) {
            if (static_cast<const PropertyNode *>(node)->propertyType() != PropertyNode::PropertyType::StandardProperty)
                generateAddendum(node, BindableProperty, marker);
        }

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
            const auto *enume = static_cast<const EnumNode *>(node);

            QSet<QString> definedItems;
            const QList<EnumItem> &items = enume->items();
            for (const auto &item : items)
                definedItems.insert(item.name());

            const auto &documentedItemList = enume->doc().enumItemNames();
            QSet<QString> documentedItems(documentedItemList.cbegin(), documentedItemList.cend());
            const QSet<QString> allItems = definedItems + documentedItems;
            if (allItems.size() > definedItems.size()
                || allItems.size() > documentedItems.size()) {
                for (const auto &it : allItems) {
                    if (!definedItems.contains(it)) {
                        QString details;
                        QString best = nearestName(it, definedItems);
                        if (!best.isEmpty() && !documentedItems.contains(best))
                            details = QStringLiteral("Maybe you meant '%1'?").arg(best);

                        node->doc().location().warning(
                                QStringLiteral("No such enum item '%1' in %2")
                                        .arg(it, node->plainFullName()),
                                details);
                    } else if (!documentedItems.contains(it)) {
                        node->doc().location().warning(
                                QStringLiteral("Undocumented enum item '%1' in %2")
                                        .arg(it, node->plainFullName()));
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
                                fn->doc().location().warning(
                                        QStringLiteral("Undocumented parameter '%1' in %2")
                                                .arg(name, node->plainFullName()));
                            }
                        }
                    }
                }
                for (const auto &name : documentedNames) {
                    if (!declaredNames.contains(name)) {
                        QString best = nearestName(name, declaredNames);
                        QString details;
                        if (!best.isEmpty())
                            details = QStringLiteral("Maybe you meant '%1'?").arg(best);
                        fn->doc().location().warning(QStringLiteral("No such parameter '%1' in %2")
                                                             .arg(name, fn->plainFullName()),
                                                     details);
                    }
                }
            }
            /*
              This return value check should be implemented
              for all functions with a return type.
              mws 13/12/2018
            */
            if (!fn->isDeprecated() && fn->returnsBool() && !fn->isMarkedReimp()
                && !fn->isOverload()) {
                if (!fn->doc().body().contains("return"))
                    node->doc().location().warning(
                            QStringLiteral("Undocumented return value "
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

    const auto *en = static_cast<const ExampleNode *>(node);
    QString exampleUrl{Config::instance().get(CONFIG_URL + Config::dot + CONFIG_EXAMPLES).asString()};

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
    QString pathRoot;
    QStringMultiMap *metaTagMap = en->doc().metaTagMap();
    if (metaTagMap)
        pathRoot = metaTagMap->value(QLatin1String("installpath"));
    if (pathRoot.isEmpty())
        pathRoot = Config::instance().get(CONFIG_EXAMPLESINSTALLPATH).asString();
    QStringList path = QStringList() << pathRoot << en->name();
    path.removeAll(QString());

    Text text;
    text << Atom::ParaLeft
         << Atom(Atom::Link, exampleUrl.replace(placeholder, path.join(separator)))
         << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << Atom(Atom::String, link)
         << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << Atom::ParaRight;

    generateText(text, nullptr, marker);
}

void Generator::addImageToCopy(const ExampleNode *en, const ResolvedFile& resolved_file)
{
    QDir dirInfo;
    // TODO: [uncentralized-output-directory-structure]
    const QString prefix("/images/used-in-examples");

    // TODO: Generators probably should not need to keep track of which files were generated.
    // Understand if we really need this information and where it should
    // belong, considering that it should be part of whichever system
    // would actually store the file itself.
    s_outFileNames << prefix.mid(1) + "/" + resolved_file.get_query();


    // TODO: [uncentralized-output-directory-structure]
    QString imgOutDir = s_outDir + prefix + "/" + QFileInfo{resolved_file.get_query()}.path();
    if (!dirInfo.mkpath(imgOutDir))
        en->location().fatal(QStringLiteral("Cannot create output directory '%1'").arg(imgOutDir));
    Config::copyFile(en->location(), resolved_file.get_path(), QFileInfo{resolved_file.get_query()}.fileName(), imgOutDir);
}

// TODO: [multi-purpose-function-with-flag][generate-file-list]
// Avoid the use of a boolean flag to dispatch to the correct
// implementation trough branching.
// We always have to process both images and files, such that we
// should consider to remove the branching altogheter, performing both
// operations in a single call.
// Otherwise, if this turns out to be infeasible, complex or
// possibly-confusing, consider extracting the processing code outside
// the function and provide two higer-level dispathing functions for
// files and images.

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

    for (const auto &path : std::as_const(paths)) {
        auto maybe_resolved_file{file_resolver.resolve(path)};
        if (!maybe_resolved_file) {
            // TODO: [uncentralized-admonition][failed-resolve-file]
            QString details = std::transform_reduce(
                file_resolver.get_search_directories().cbegin(),
                file_resolver.get_search_directories().cend(),
                u"Searched directories:"_s,
                std::plus(),
                [](const DirectoryPath &directory_path) -> QString { return u' ' + directory_path.value(); }
            );

            en->location().warning(u"(Generator)Cannot find file to quote from: %1"_s.arg(path), details);

            continue;
        }

        auto file{*maybe_resolved_file};
        if (images) addImageToCopy(en, file);
        else        generateExampleFilePage(en, file, marker);

        openedList.next();
        text << Atom(Atom::ListItemNumber, openedList.numberString())
             << Atom(Atom::ListItemLeft, openedList.styleString()) << Atom::ParaLeft
             << Atom(atomType, file.get_query()) << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << file.get_query()
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << Atom::ParaRight
             << Atom(Atom::ListItemRight, openedList.styleString());
    }
    text << Atom(Atom::ListRight, openedList.styleString());
    if (!paths.isEmpty())
        generateText(text, en, marker);
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
    if (node->isInternal() && !m_showInternal)
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
              A collection node collects: groups, C++ modules, or QML
              modules. Testing for a CollectionNode must be done
              before testing for a TextPageNode because a
              CollectionNode is a PageNode at this point.

              Don't output an HTML page for the collection node unless
              the \group, \module, or \qmlmodule command was actually
              seen by qdoc in the qdoc comment for the node.

              A key prerequisite in this case is the call to
              mergeCollections(cn). We must determine whether this
              group, module or QML module has members in other
              modules. We know at this point that cn's members list
              contains only members in the current module. Therefore,
              before outputting the page for cn, we must search for
              members of cn in the other modules and add them to the
              members list.
            */
            auto *cn = static_cast<CollectionNode *>(node);
            if (cn->wasSeen()) {
                m_qdb->mergeCollections(cn);
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
            } else if (node->isQmlType()) {
                beginSubPage(node, fileName(node));
                auto *qcn = static_cast<QmlTypeNode *>(node);
                generateQmlTypePage(qcn, marker);
                endSubPage();
            } else if (node->isProxyNode()) {
                beginSubPage(node, fileName(node));
                generateProxyPage(static_cast<Aggregate *>(node), marker);
                endSubPage();
            }
        }
    }

    if (node->isAggregate()) {
        auto *aggregate = static_cast<Aggregate *>(node);
        const NodeList &children = aggregate->childNodes();
        for (auto *child : children) {
            if (child->isPageNode() && !child->isPrivate()) {
                generateDocumentation(child);
            } else if (!node->parent() && child->isInAPI() && !child->isRelatedNonmember()) {
                // Warn if there are documented non-page-generating nodes in the root namespace
                child->location().warning(u"No documentation generated for %1 '%2' in global scope."_s
                    .arg(typeString(child), child->name()),
                            u"Maybe you forgot to use the '\\relates' command?"_s);
                child->setStatus(Node::DontDocument);
            }
        }
    }
}

void Generator::generateReimplementsClause(const FunctionNode *fn, CodeMarker *marker)
{
    if (fn->overridesThis().isEmpty() || !fn->parent()->isClassNode())
        return;

    auto *cn = static_cast<ClassNode *>(fn->parent());
    const FunctionNode *overrides = cn->findOverriddenFunction(fn);
    if (overrides && !overrides->isPrivate() && !overrides->parent()->isPrivate()) {
        if (overrides->hasDoc()) {
            Text text;
            text << Atom::ParaLeft << "Reimplements: ";
            QString fullName =
                    overrides->parent()->name()
                    + "::" + overrides->signature(Node::SignaturePlain);
            appendFullName(text, overrides->parent(), fullName, overrides);
            text << "." << Atom::ParaRight;
            generateText(text, fn, marker);
        } else {
            fn->doc().location().warning(
                    QStringLiteral("Illegal \\reimp; no documented virtual function for %1")
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

QString Generator::formatSince(const Node *node)
{
    QStringList since = node->since().split(QLatin1Char(' '));

    // If there is only one argument, assume it is the Qt version number.
    if (since.size() == 1)
        return "Qt " + since[0];

    // Otherwise, use the original <project> <version> string.
    return node->since();
}

/*!
    \internal
    Returns a string representing status information of a \a node.

    If a status description is returned, it is one of:
    \list
      \li Custom status set explicitly in node's documentation using
          \c {\meta {status} {<description>}},
      \li 'Deprecated [since <version>]' (\\deprecated [<version>]),
      \li 'Preliminary' (\\preliminary), or
      \li The description adopted from associated module's state:
          \c {\modulestate {<description>}}.
    \endlist

    Otherwise, returns \c std::nullopt.
*/
std::optional<QString> formatStatus(const Node *node, QDocDatabase *qdb)
{
    QString status;

    if (const auto metaMap = node->doc().metaTagMap(); metaMap) {
        status = metaMap->value("status");
        if (!status.isEmpty())
            return {status};
    }
    if (node->status() == Node::Deprecated) {
        status = u"Deprecated"_s;
        if (const auto since = node->deprecatedSince(); !since.isEmpty())
            status += " since %1"_L1.arg(since);
    } else if (node->status() == Node::Preliminary) {
        status = u"Preliminary"_s;
    } else if (const auto collection = qdb->getModuleNode(node); collection) {
        status = collection->state();
    }

    return status.isEmpty() ? std::nullopt : std::optional(status);
}

void Generator::generateSince(const Node *node, CodeMarker *marker)
{
    if (!node->since().isEmpty()) {
        Text text;
        text << Atom::ParaLeft << "This " << typeString(node) << " was introduced in "
             << formatSince(node) << "." << Atom::ParaRight;
        generateText(text, node, marker);
    }
}

void Generator::generateNoexceptNote(const Node* node, CodeMarker* marker) {
    std::vector<const Node*> nodes;
    if (node->isSharedCommentNode()) {
        auto shared_node = static_cast<const SharedCommentNode*>(node);
        nodes.reserve(shared_node->collective().size());
        nodes.insert(nodes.begin(), shared_node->collective().begin(), shared_node->collective().end());
    } else nodes.push_back(node);

    std::size_t counter{1};
    for (const Node* node : nodes) {
        if (node->isFunction(Node::CPP)) {
            if (auto exception_info = static_cast<const FunctionNode*>(node)->getNoexcept(); exception_info && !(*exception_info).isEmpty()) {
                Text text;
                text << Atom::NoteLeft
                        << (nodes.size() > 1 ? QString::fromStdString(" ("s + std::to_string(counter) + ")"s) : QString::fromStdString("This ") + typeString(node))
                        << " does not throw any exception when " << "\"" << *exception_info << "\"" << " is true."
                    << Atom::NoteRight;
                generateText(text, node, marker);
            }
        }

        ++counter;
    }
}

void Generator::generateStatus(const Node *node, CodeMarker *marker)
{
    Text text;

    switch (node->status()) {
    case Node::Active:
        // Output the module 'state' description if set.
        if (node->isModule() || node->isQmlModule()) {
            const QString &state = static_cast<const CollectionNode*>(node)->state();
            if (!state.isEmpty()) {
                text << Atom::ParaLeft << "This " << typeString(node) << " is in "
                     << Atom(Atom::FormattingLeft, ATOM_FORMATTING_ITALIC) << state
                     << Atom(Atom::FormattingRight, ATOM_FORMATTING_ITALIC) << " state."
                     << Atom::ParaRight;
            }
        }
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
        text << "This " << typeString(node) << " is deprecated";
        if (const QString &version = node->deprecatedSince(); !version.isEmpty())
            text << " since " << version;
        text << ". We strongly advise against using it in new code.";
        if (node->isAggregate())
            text << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);
        text << Atom::ParaRight;
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
    text << Atom(Atom::DivLeft,
            "class=\"admonition %1\""_L1.arg(generateNote ? u"note"_s : u"auto"_s));
    text << Atom::ParaLeft;

    if (generateNote) {
        text  << Atom(Atom::FormattingLeft, ATOM_FORMATTING_BOLD)
              << "Note: " << Atom(Atom::FormattingRight, ATOM_FORMATTING_BOLD);
    }

    switch (type) {
    case Invokable:
        text << "This function can be invoked via the meta-object system and from QML. See "
             << Atom(Atom::AutoLink, "Q_INVOKABLE")
             << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK) << ".";
        break;
    case PrivateSignal:
        text << "This is a private signal. It can be used in signal connections "
                "but cannot be emitted by the user.";
        break;
    case QmlSignalHandler:
    {
        QString handler(node->name());
        qsizetype prefixLocation = handler.lastIndexOf('.', -2) + 1;
        handler[prefixLocation] = handler[prefixLocation].toTitleCase();
        handler.insert(prefixLocation, QLatin1String("on"));
        text << "The corresponding handler is "
             << Atom(Atom::FormattingLeft, ATOM_FORMATTING_TELETYPE) << handler
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_TELETYPE) << ".";
        break;
    }
    case AssociatedProperties:
    {
        if (!node->isFunction())
            return;
        const auto *fn = static_cast<const FunctionNode *>(node);
        auto nodes = fn->associatedProperties();
        if (nodes.isEmpty())
            return;
        std::sort(nodes.begin(), nodes.end(), Node::nodeNameLessThan);
        for (const auto *n : std::as_const(nodes)) {
            QString msg;
            const auto *pn = static_cast<const PropertyNode *>(n);
            switch (pn->role(fn)) {
            case PropertyNode::FunctionRole::Getter:
                msg = QStringLiteral("Getter function");
                break;
            case PropertyNode::FunctionRole::Setter:
                msg = QStringLiteral("Setter function");
                break;
            case PropertyNode::FunctionRole::Resetter:
                msg = QStringLiteral("Resetter function");
                break;
            case PropertyNode::FunctionRole::Notifier:
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
    case BindableProperty:
    {
        text << "This property supports "
             << Atom(Atom::Link, "QProperty")
             << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK) << "QProperty"
             << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
        text << " bindings.";
        break;
    }
    default:
        return;
    }

    text << Atom::ParaRight
         << Atom::DivRight;
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
        if (!child->isDeprecated()) {
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
    s_currentGenerator = this;
    generateDocumentation(m_qdb->primaryTreeRoot());
}

Generator *Generator::generatorForFormat(const QString &format)
{
    for (const auto &generator : std::as_const(s_generators)) {
        if (generator->format() == format)
            return generator;
    }
    return nullptr;
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
    QStringList result;
    QStringMultiMap *metaTagMap = inner->doc().metaTagMap();
    if (metaTagMap)
        result = metaTagMap->values(t);
    if (!result.isEmpty())
        metaTagMap->remove(t);
    return result;
}

QString Generator::indent(int level, const QString &markedCode)
{
    if (level == 0)
        return markedCode;

    QString t;
    int column = 0;

    int i = 0;
    while (i < markedCode.size()) {
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
    s_outputFormats = config.getOutputFormats();
    s_redirectDocumentationToDevNull = config.get(CONFIG_REDIRECTDOCUMENTATIONTODEVNULL).asBool();

    for (auto &g : s_generators) {
        if (s_outputFormats.contains(g->format())) {
            s_currentGenerator = g;
            g->initializeGenerator();
        }
    }

    const auto &configFormatting = config.subVars(CONFIG_FORMATTING);
    for (const auto &n : configFormatting) {
        QString formattingDotName = CONFIG_FORMATTING + Config::dot + n;
        const auto &formattingDotNames = config.subVars(formattingDotName);
        for (const auto &f : formattingDotNames) {
            const auto &configVar = config.get(formattingDotName + Config::dot + f);
            QString def{configVar.asString()};
            if (!def.isEmpty()) {
                int numParams = Config::numParams(def);
                int numOccs = def.count("\1");
                if (numParams != 1) {
                    configVar.location().warning(QStringLiteral("Formatting '%1' must "
                                                                "have exactly one "
                                                                "parameter (found %2)")
                                                                .arg(n, numParams));
                } else if (numOccs > 1) {
                    configVar.location().fatal(QStringLiteral("Formatting '%1' must "
                                             "contain exactly one "
                                             "occurrence of '\\1' "
                                             "(found %2)")
                                            .arg(n, numOccs));
                } else {
                    int paramPos = def.indexOf("\1");
                    s_fmtLeftMaps[f].insert(n, def.left(paramPos));
                    s_fmtRightMaps[f].insert(n, def.mid(paramPos + 1));
                }
            }
        }
    }

    s_project = config.get(CONFIG_PROJECT).asString();
    s_outDir = config.getOutputDir();
    s_outSubdir = s_outDir.mid(s_outDir.lastIndexOf('/') + 1);

    s_outputPrefixes.clear();
    QStringList items{config.get(CONFIG_OUTPUTPREFIXES).asStringList()};
    if (!items.isEmpty()) {
        for (const auto &prefix : items)
            s_outputPrefixes[prefix] =
                    config.get(CONFIG_OUTPUTPREFIXES + Config::dot + prefix).asString();
    } else {
        s_outputPrefixes[QLatin1String("QML")] = QLatin1String("qml-");
    }

    s_outputSuffixes.clear();
    for (const auto &suffix : config.get(CONFIG_OUTPUTSUFFIXES).asStringList())
        s_outputSuffixes[suffix] = config.get(CONFIG_OUTPUTSUFFIXES
                                              + Config::dot + suffix).asString();

    s_noLinkErrors = config.get(CONFIG_NOLINKERRORS).asBool();
    s_autolinkErrors = config.get(CONFIG_AUTOLINKERRORS).asBool();
}

/*!
  Creates template-specific subdirs (e.g. /styles and /scripts for HTML)
  and copies the files to them.
  */
void Generator::copyTemplateFiles(const QString &configVar, const QString &subDir)
{
    // TODO: [resolving-files-unlinked-to-doc]
    // This is another case of resolving files, albeit it doesn't use Doc::resolveFile.
    // While it may be left out of a first iteration of the file
    // resolution logic, it should later be integrated into it.
    // This should come naturally when the output directory logic is
    // extracted and copying a file should require a validated
    // intermediate format.
    // Do note that what is done here is a bit different from the
    // resolve file routine that is done for other user-given paths.
    // Thas is, the paths will always be absolute and not relative as
    // they are resolved from the configuration.
    // Ideally, this could be solved in the configuration already,
    // together with the other configuration resolution processes that
    // do not abide by the same constraints that, for example, snippet
    // resolution uses.
    Config &config = Config::instance();
    QStringList files = config.getCanonicalPathList(configVar, Config::Validate);
    const auto &loc = config.get(configVar).location();
    if (!files.isEmpty()) {
        QDir dirInfo;
        // TODO: [uncentralized-output-directory-structure]
        // As with other places in the generation pass, the details of
        // where something is saved in the output directory are spread
        // to whichever part of the generation does the saving.
        // It is hence complex to build a model of how an output
        // directory looks like, as the knowledge has no specific
        // entry point or chain-path that can be followed in full.
        // Each of those operations should be centralized in a system
        // that uniquely knows what the format of the output-directory
        // is and how to perform operations on it.
        // Later, move this operation to that centralized system.
        QString templateDir = s_outDir + QLatin1Char('/') + subDir;
        if (!dirInfo.exists(templateDir) && !dirInfo.mkdir(templateDir)) {
            // TODO: [uncentralized-admonition]
            loc.fatal(QStringLiteral("Cannot create %1 directory '%2'").arg(subDir, templateDir));
        } else {
            for (const auto &file : files) {
                if (!file.isEmpty())
                    Config::copyFile(loc, file, file, templateDir);
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
    s_outFileNames.clear();
    s_useOutputSubdirs = true;
    if (config.get(format() + Config::dot + "nosubdirs").asBool())
        resetUseOutputSubdirs();

    if (s_outputFormats.isEmpty())
        return;

    s_outDir = config.getOutputDir(format());
    if (s_outDir.isEmpty()) {
        Location().fatal(QStringLiteral("No output directory specified in "
                                        "configuration file or on the command line"));
    } else {
        s_outSubdir = s_outDir.mid(s_outDir.lastIndexOf('/') + 1);
    }

    QDir outputDir(s_outDir);
    if (outputDir.exists()) {
        if (!config.generating() && Generator::useOutputSubdirs()) {
            if (!outputDir.isEmpty())
                Location().error(QStringLiteral("Output directory '%1' exists but is not empty")
                                .arg(s_outDir));
        }
    } else if (!outputDir.mkpath(QStringLiteral("."))) {
        Location().fatal(QStringLiteral("Cannot create output directory '%1'").arg(s_outDir));
    }

    // Output directory exists, which is enough for prepare phase.
    if (config.preparing())
        return;

    const QLatin1String imagesDir("images");
    if (!outputDir.exists(imagesDir) && !outputDir.mkdir(imagesDir))
        Location().fatal(QStringLiteral("Cannot create images directory '%1'").arg(outputDir.filePath(imagesDir)));

    copyTemplateFiles(format() + Config::dot + CONFIG_STYLESHEETS, "style");
    copyTemplateFiles(format() + Config::dot + CONFIG_SCRIPTS, "scripts");
    copyTemplateFiles(format() + Config::dot + CONFIG_EXTRAIMAGES, "images");

    // Use a format-specific .quotinginformation if defined, otherwise a global value
    if (config.subVars(format()).contains(CONFIG_QUOTINGINFORMATION))
        m_quoting = config.get(format() + Config::dot + CONFIG_QUOTINGINFORMATION).asBool();
    else
        m_quoting = config.get(CONFIG_QUOTINGINFORMATION).asBool();
}

/*!
  Updates the generator's m_showInternal from the Config.
 */
void Generator::initializeGenerator()
{
    m_showInternal = Config::instance().showInternal();
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
    // Prefix is applied to QML types
    if (node->isQmlType())
        return s_outputPrefixes[QLatin1String("QML")];

    return QString();
}

QString Generator::outputSuffix(const Node *node)
{
    // Suffix is applied to QML types, as
    // well as module pages.
    if (node->isQmlModule() || node->isQmlType())
        return s_outputSuffixes[QLatin1String("QML")];

    return QString();
}

bool Generator::parseArg(const QString &src, const QString &tag, int *pos, int n,
                         QStringView *contents, QStringView *par1)
{
#define SKIP_CHAR(c)                                                                               \
    if (i >= n || src[i] != c)                                                                     \
        return false;                                                                              \
    ++i;

#define SKIP_SPACE                                                                                 \
    while (i < n && src[i] == ' ')                                                                 \
        ++i;

    qsizetype i = *pos;
    qsizetype j {};

    // assume "<@" has been parsed outside
    // SKIP_CHAR('<');
    // SKIP_CHAR('@');

    if (tag != QStringView(src).mid(i, tag.size())) {
        return false;
    }

    // skip tag
    i += tag.size();

    // parse stuff like:  linkTag("(<@link node=\"([^\"]+)\">).*(</@link>)");
    if (par1) {
        SKIP_SPACE;
        // read parameter name
        j = i;
        while (i < n && src[i].isLetter())
            ++i;
        if (src[i] == '=') {
            SKIP_CHAR('=');
            SKIP_CHAR('"');
            // skip parameter name
            j = i;
            while (i < n && src[i] != '"')
                ++i;
            *par1 = QStringView(src).mid(j, i - j);
            SKIP_CHAR('"');
            SKIP_SPACE;
        }
    }
    SKIP_SPACE;
    SKIP_CHAR('>');

    // find contents up to closing "</@tag>
    j = i;
    for (; true; ++i) {
        if (i + 4 + tag.size() > n)
            return false;
        if (src[i] != '<')
            continue;
        if (src[i + 1] != '/')
            continue;
        if (src[i + 2] != '@')
            continue;
        if (tag != QStringView(src).mid(i + 3, tag.size()))
            continue;
        if (src[i + 3 + tag.size()] != '>')
            continue;
        break;
    }

    *contents = QStringView(src).mid(j, i - j);

    i += tag.size() + 4;

    *pos = i;
    return true;
#undef SKIP_CHAR
#undef SKIP_SPACE
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
    m_inLink = false;
    m_inContents = false;
    m_inSectionHeading = false;
    m_inTableHeader = false;
    m_numTableRows = 0;
    m_threeColumnEnumValueTable = true;
    m_link.clear();
    m_sectionNumber.clear();
}

void Generator::supplementAlsoList(const Node *node, QList<Text> &alsoList)
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

            if (alternateFunc && alternateFunc->access() != Access::Private) {
                int i;
                for (i = 0; i < alsoList.size(); ++i) {
                    if (alsoList.at(i).toString().contains(alternateName))
                        break;
                }

                if (i == alsoList.size()) {
                    if (alternateFunc->isDeprecated() && !fn->isDeprecated())
                        return;
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
    for (const auto &generator : std::as_const(s_generators)) {
        if (s_outputFormats.contains(generator->format()))
            generator->terminateGenerator();
    }

    // REMARK: Generators currently, due to recent changes and the
    // transitive nature of the current codebase, receive some of
    // their dependencies in the constructor and some of them in their
    // initialize-terminate lifetime.
    // This means that generators need to be constructed and
    // destructed between usages such that if multiple usages are
    // required, the generators present in the list will have been
    // destroyed by then such that accessing them would be an error.
    // The current codebase calls initialize and the correspective
    // terminate with the same scope as the lifetime of the
    // generators.
    // Then, clearing the list ensures that, if another generator
    // execution is needed, the stale generators will not be removed
    // as to be replaced by newly constructed ones.
    // Do note that it is not clear that this will happen for any call
    // in Qt's documentation and this should work only because of the
    // form of the current codebase and the scoping of the
    // initialize-terminate calls. As such, this should be considered
    // a patchwork that may or may not be doing anything and that may
    // break due to changes in other parts of the codebase.
    //
    // This is still to be considered temporary as the whole
    // initialize-terminate idiom must be removed from the codebase.
    s_generators.clear();

    s_fmtLeftMaps.clear();
    s_fmtRightMaps.clear();
    s_outDir.clear();
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
    while (trimmed.size() > 0 && trimmed[trimmed.size() - 1].isSpace())
        trimmed.truncate(trimmed.size() - 1);

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
    case Node::QmlValueType:
        return "type";
    case Node::Page:
        return "documentation";
    case Node::Enum:
        return "enum";
    case Node::Typedef:
    case Node::TypeAlias:
        return "typedef";
    case Node::Function: {
        const auto fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::QmlSignal:
            return "signal";
        case FunctionNode::QmlSignalHandler:
            return "signal handler";
        case FunctionNode::QmlMethod:
            return "method";
        case FunctionNode::MacroWithParams:
        case FunctionNode::MacroWithoutParams:
            return "macro";
        default:
            break;
        }
        return "function";
    }
    case Node::Property:
    case Node::QmlProperty:
        return "property";
    case Node::Module:
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
    Location::internalError(QStringLiteral("unknown atom type '%1' in %2 generator")
                                    .arg(atom->typeString(), format()));
}

QT_END_NAMESPACE
