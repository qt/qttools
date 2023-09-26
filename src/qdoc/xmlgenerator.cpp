// Copyright (C) 2019 Thibaut Cuvelier
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "xmlgenerator.h"

#include "enumnode.h"
#include "examplenode.h"
#include "functionnode.h"
#include "qdocdatabase.h"
#include "typedefnode.h"

QT_BEGIN_NAMESPACE

const QRegularExpression XmlGenerator::m_funcLeftParen(QStringLiteral("^\\S+(\\(.*\\))$"));

XmlGenerator::XmlGenerator(FileResolver& file_resolver) : Generator(file_resolver) {}

/*!
  Do not display \brief for QML types, document and collection nodes
 */
bool XmlGenerator::hasBrief(const Node *node)
{
    return !(node->isQmlType() || node->isPageNode() || node->isCollectionNode());
}

/*!
  Determines whether the list atom should be shown with three columns
  (constant-value-description).
 */
bool XmlGenerator::isThreeColumnEnumValueTable(const Atom *atom)
{
    while (atom && !(atom->type() == Atom::ListRight && atom->string() == ATOM_LIST_VALUE)) {
        if (atom->type() == Atom::ListItemLeft && !matchAhead(atom, Atom::ListItemRight))
            return true;
        atom = atom->next();
    }
    return false;
}

/*!
  Determines whether the list atom should be shown with just one column (value).
 */
bool XmlGenerator::isOneColumnValueTable(const Atom *atom)
{
    if (atom->type() != Atom::ListLeft || atom->string() != ATOM_LIST_VALUE)
        return false;

    while (atom && atom->type() != Atom::ListTagRight)
        atom = atom->next();

    if (atom) {
        if (!matchAhead(atom, Atom::ListItemLeft))
            return false;
        if (!atom->next())
            return false;
        return matchAhead(atom->next(), Atom::ListItemRight);
    }
    return false;
}

/*!
  Header offset depending on the type of the node
 */
int XmlGenerator::hOffset(const Node *node)
{
    switch (node->nodeType()) {
    case Node::Namespace:
    case Node::Class:
    case Node::Struct:
    case Node::Union:
    case Node::Module:
        return 2;
    case Node::QmlModule:
    case Node::QmlValueType:
    case Node::QmlType:
    case Node::Page:
    case Node::Group:
        return 1;
    case Node::Enum:
    case Node::TypeAlias:
    case Node::Typedef:
    case Node::Function:
    case Node::Property:
    default:
        return 3;
    }
}

/*!
  Rewrites the brief of this node depending on its first word.
  Only for properties and variables (does nothing otherwise).
 */
void XmlGenerator::rewritePropertyBrief(const Atom *atom, const Node *relative)
{
    if (relative->nodeType() != Node::Property && relative->nodeType() != Node::Variable)
        return;
    atom = atom->next();
    if (!atom || atom->type() != Atom::String)
        return;

    const QString firstWord =
            atom->string().toLower().section(' ', 0, 0, QString::SectionSkipEmpty);
    const QStringList words{ "the", "a", "an", "whether", "which" };
    if (words.contains(firstWord)) {
        QString str = QLatin1String("This ")
                + QLatin1String(relative->nodeType() == Node::Property ? "property" : "variable")
                + QLatin1String(" holds ") + atom->string().left(1).toLower()
                + atom->string().mid(1);
        const_cast<Atom *>(atom)->setString(str);
    }
}

/*!
  Returns the type of this atom as an enumeration.
 */
Node::NodeType XmlGenerator::typeFromString(const Atom *atom)
{
    const auto &name = atom->string();
    if (name.startsWith(QLatin1String("qml")))
        return Node::QmlModule;
    else if (name.startsWith(QLatin1String("groups")))
        return Node::Group;
    else
        return Node::Module;
}

/*!
  For images shown in examples, set the image file to the one it
  will have once the documentation is generated.
 */
void XmlGenerator::setImageFileName(const Node *relative, const QString &fileName)
{
    if (relative->isExample()) {
        const auto cen = static_cast<const ExampleNode *>(relative);
        if (cen->imageFileName().isEmpty()) {
            auto *en = const_cast<ExampleNode *>(cen);
            en->setImageFileName(fileName);
        }
    }
}

/*!
  Handles the differences in lists between list tags and since tags, and
  returns the content of the list entry \a atom (first member of the pair).
  It also returns the number of items to skip ahead (second member of the pair).
 */
std::pair<QString, int> XmlGenerator::getAtomListValue(const Atom *atom)
{
    const Atom *lookAhead = atom->next();
    if (!lookAhead)
        return std::pair<QString, int>(QString(), 1);

    QString t = lookAhead->string();
    lookAhead = lookAhead->next();
    if (!lookAhead || lookAhead->type() != Atom::ListTagRight)
        return std::pair<QString, int>(QString(), 1);

    lookAhead = lookAhead->next();
    int skipAhead;
    if (lookAhead && lookAhead->type() == Atom::SinceTagLeft) {
        lookAhead = lookAhead->next();
        Q_ASSERT(lookAhead && lookAhead->type() == Atom::String);
        t += QLatin1String(" (since ");
        if (lookAhead->string().at(0).isDigit())
            t += QLatin1String("Qt ");
        t += lookAhead->string() + QLatin1String(")");
        skipAhead = 4;
    } else {
        skipAhead = 1;
    }
    return std::pair<QString, int>(t, skipAhead);
}

/*!
  Parses the table attributes from the given \a atom.
  This method returns a pair containing the width (%) and
  the attribute for this table (either "generic" or
  "borderless").
 */
std::pair<QString, QString> XmlGenerator::getTableWidthAttr(const Atom *atom)
{
    QString p0, p1;
    QString attr = "generic";
    QString width;
    if (atom->count() > 0) {
        p0 = atom->string(0);
        if (atom->count() > 1)
            p1 = atom->string(1);
    }
    if (!p0.isEmpty()) {
        if (p0 == QLatin1String("borderless"))
            attr = p0;
        else if (p0.contains(QLatin1Char('%')))
            width = p0;
    }
    if (!p1.isEmpty()) {
        if (p1 == QLatin1String("borderless"))
            attr = p1;
        else if (p1.contains(QLatin1Char('%')))
            width = p1;
    }

    // Many times, in the documentation, there is a space before the % sign:
    // this breaks the parsing logic above.
    if (width == QLatin1String("%")) {
        // The percentage is typically stored in p0, parse it as an int.
        bool ok = false;
        int widthPercentage = p0.toInt(&ok);
        if (ok) {
            width = QString::number(widthPercentage) + "%";
        } else {
            width = {};
        }
    }

    return {width, attr};
}

/*!
  Registers an anchor reference and returns a unique
  and cleaned copy of the reference (the one that should be
  used in the output).
  To ensure unicity throughout the document, this method
  uses the \a refMap cache.
 */
QString XmlGenerator::registerRef(const QString &ref, bool xmlCompliant)
{
    QString cleanRef = Generator::cleanRef(ref, xmlCompliant);

    for (;;) {
        QString &prevRef = refMap[cleanRef.toLower()];
        if (prevRef.isEmpty()) {
            // This reference has never been met before for this document: register it.
            prevRef = ref;
            break;
        } else if (prevRef == ref) {
            // This exact same reference was already found. This case typically occurs within refForNode.
            break;
        }
        cleanRef += QLatin1Char('x');
    }
    return cleanRef;
}

/*!
  Generates a clean and unique reference for the given \a node.
  This reference may depend on the type of the node (typedef,
  QML signal, etc.)
 */
QString XmlGenerator::refForNode(const Node *node)
{
    QString ref;
    switch (node->nodeType()) {
    case Node::Enum:
        ref = node->name() + "-enum";
        break;
    case Node::Typedef: {
        const auto *tdf = static_cast<const TypedefNode *>(node);
        if (tdf->associatedEnum())
            return refForNode(tdf->associatedEnum());
    } Q_FALLTHROUGH();
    case Node::TypeAlias:
        ref = node->name() + "-typedef";
        break;
    case Node::Function: {
        const auto fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::QmlSignal:
            ref = fn->name() + "-signal";
            break;
        case FunctionNode::QmlSignalHandler:
            ref = fn->name() + "-signal-handler";
            break;
        case FunctionNode::QmlMethod:
            ref = fn->name() + "-method";
            if (fn->overloadNumber() != 0)
                ref += QLatin1Char('-') + QString::number(fn->overloadNumber());
            break;
        default:
            if (fn->hasOneAssociatedProperty() && fn->doc().isEmpty()) {
                return refForNode(fn->associatedProperties()[0]);
            } else {
                ref = fn->name();
                if (fn->overloadNumber() != 0)
                    ref += QLatin1Char('-') + QString::number(fn->overloadNumber());
            }
            break;
        }
    } break;
    case Node::SharedComment: {
        if (!node->isPropertyGroup())
            break;
    } Q_FALLTHROUGH();
    case Node::QmlProperty:
        if (node->isAttached())
            ref = node->name() + "-attached-prop";
        else
            ref = node->name() + "-prop";
        break;
    case Node::Property:
        ref = node->name() + "-prop";
        break;
    case Node::Variable:
        ref = node->name() + "-var";
        break;
    default:
        break;
    }
    return registerRef(ref);
}

/*!
  Construct the link string for the \a node and return it.
  The \a relative node is used to decide whether the link
  we are generating is in the same file as the target.
  Note the relative node can be 0, which pretty much
  guarantees that the link and the target aren't in the
  same file.
  */
QString XmlGenerator::linkForNode(const Node *node, const Node *relative)
{
    if (node == nullptr)
        return QString();
    if (!node->url().isNull())
        return node->url();
    if (fileBase(node).isEmpty())
        return QString();
    if (node->isPrivate())
        return QString();

    QString fn = fileName(node);
    if (node->parent() && node->parent()->isQmlType() && node->parent()->isAbstract()) {
        if (Generator::qmlTypeContext()) {
            if (Generator::qmlTypeContext()->inherits(node->parent())) {
                fn = fileName(Generator::qmlTypeContext());
            } else if (node->parent()->isInternal() && !noLinkErrors()) {
                node->doc().location().warning(
                        QStringLiteral("Cannot link to property in internal type '%1'")
                                .arg(node->parent()->name()));
                return QString();
            }
        }
    }

    QString link = fn;

    if (!node->isPageNode() || node->isPropertyGroup()) {
        QString ref = refForNode(node);
        if (relative && fn == fileName(relative) && ref == refForNode(relative))
            return QString();

        link += QLatin1Char('#');
        link += ref;
    }

    /*
      If the output is going to subdirectories, then if the
      two nodes will be output to different directories, then
      the link must go up to the parent directory and then
      back down into the other subdirectory.
     */
    if (relative && (node != relative)) {
        if (useOutputSubdirs() && !node->isExternalPage()
            && node->outputSubdirectory() != relative->outputSubdirectory()) {
            if (link.startsWith(QString(node->outputSubdirectory() + QLatin1Char('/')))) {
                link.prepend(QString("../"));
            } else {
                link.prepend(QString("../" + node->outputSubdirectory() + QLatin1Char('/')));
            }
        }
    }
    return link;
}

/*!
  This function is called for links, i.e. for words that
  are marked with the qdoc link command. For autolinks
  that are not marked with the qdoc link command, the
  getAutoLink() function is called

  It returns the string for a link found by using the data
  in the \a atom to search the database. It also sets \a node
  to point to the target node for that link. \a relative points
  to the node holding the qdoc comment where the link command
  was found.
 */
QString XmlGenerator::getLink(const Atom *atom, const Node *relative, const Node **node)
{
    const QString &t = atom->string();

    if (t.isEmpty())
        return t;

    if (t.at(0) == QChar('h')) {
        if (t.startsWith("http:") || t.startsWith("https:"))
            return t;
    } else if (t.at(0) == QChar('f')) {
        if (t.startsWith("file:") || t.startsWith("ftp:"))
            return t;
    } else if (t.at(0) == QChar('m')) {
        if (t.startsWith("mailto:"))
            return t;
    }
    return getAutoLink(atom, relative, node);
}

/*!
  This function is called for autolinks, i.e. for words that
  are not marked with the qdoc link command that qdoc has
  reason to believe should be links.

  It returns the string for a link found by using the data
  in the \a atom to search the database. It also sets \a node
  to point to the target node for that link. \a relative points
  to the node holding the qdoc comment where the link command
  was found.
 */
QString XmlGenerator::getAutoLink(const Atom *atom, const Node *relative, const Node **node,
                                  Node::Genus genus)
{
    QString ref;

    *node = m_qdb->findNodeForAtom(atom, relative, ref, genus);
    if (!(*node))
        return QString();

    QString link = (*node)->url();
    if (link.isNull()) {
        link = linkForNode(*node, relative);
    } else if (link.isEmpty()) {
        return link; // Explicit empty url (node is ignored as a link target)
    }
    if (!ref.isEmpty()) {
        qsizetype hashtag = link.lastIndexOf(QChar('#'));
        if (hashtag != -1)
            link.truncate(hashtag);
        link += QLatin1Char('#') + ref;
    }
    return link;
}

std::pair<QString, QString> XmlGenerator::anchorForNode(const Node *node)
{
    std::pair<QString, QString> anchorPair;

    anchorPair.first = Generator::fileName(node);
    if (node->isTextPageNode())
        anchorPair.second = node->title();

    return anchorPair;
}

/*!
  Returns a string describing the \a node type.
 */
QString XmlGenerator::targetType(const Node *node)
{
    if (!node)
        return QStringLiteral("external");

    switch (node->nodeType()) {
    case Node::Namespace:
        return QStringLiteral("namespace");
    case Node::Class:
    case Node::Struct:
    case Node::Union:
        return QStringLiteral("class");
    case Node::Page:
    case Node::Example:
        return QStringLiteral("page");
    case Node::Enum:
        return QStringLiteral("enum");
    case Node::TypeAlias:
        return QStringLiteral("alias");
    case Node::Typedef:
        return QStringLiteral("typedef");
    case Node::Property:
        return QStringLiteral("property");
    case Node::Function:
        return QStringLiteral("function");
    case Node::Variable:
        return QStringLiteral("variable");
    case Node::Module:
        return QStringLiteral("module");
    default:
        break;
    }
    return QString();
}

QT_END_NAMESPACE
