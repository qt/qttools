// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "node.h"

#include "aggregate.h"
#include "codemarker.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "generator.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "qmlpropertynode.h"
#include "relatedclass.h"
#include "sharedcommentnode.h"
#include "tokenizer.h"
#include "tree.h"

#include <QtCore/quuid.h>
#include <QtCore/qversionnumber.h>

#include <utility>

QT_BEGIN_NAMESPACE

/*!
  \class Node
  \brief The Node class is the base class for all the nodes in QDoc's parse tree.

  Class Node is the base class of all the node subclasses. There is a subclass of Node
  for each type of entity that QDoc can document. The types of entities that QDoc can
  document are listed in the enum type NodeType.

  After ClangCodeParser has parsed all the header files to build its precompiled header,
  it then visits the clang Abstract Syntax Tree (AST). For each node in the AST that it
  determines is in the public API and must be documented, it creates an instance of one
  of the Node subclasses and adds it to the QDoc Tree.

  Each instance of a sublass of Node has a parent pointer to link it into the Tree. The
  parent pointer is obtained by calling \l {parent()}, which returns a pointer to an
  instance of the Node subclass, Aggregate, which is never instantiated directly, but
  as the base class for certain subclasses of Node that can have children. For example,
  ClassNode and QmlTypeNode can have children, so they both inherit Aggregate, while
  PropertyNode and QmlPropertyNode can not have children, so they both inherit Node.

  \sa Aggregate, ClassNode, PropertyNode
 */

/*!
  Returns \c true if the node \a n1 is less than node \a n2. The
  comparison is performed by comparing properties of the nodes
  in order of increasing complexity.
 */
bool Node::nodeNameLessThan(const Node *n1, const Node *n2)
{
#define LT_RETURN_IF_NOT_EQUAL(a, b)                                                               \
    if ((a) != (b))                                                                                \
        return (a) < (b);

    if (n1->isPageNode() && n2->isPageNode()) {
        LT_RETURN_IF_NOT_EQUAL(n1->fullName(), n2->fullName());
        LT_RETURN_IF_NOT_EQUAL(n1->fullTitle(), n2->fullTitle());
    }

    if (n1->isFunction() && n2->isFunction()) {
        const auto *f1 = static_cast<const FunctionNode *>(n1);
        const auto *f2 = static_cast<const FunctionNode *>(n2);

        LT_RETURN_IF_NOT_EQUAL(f1->isConst(), f2->isConst());
        LT_RETURN_IF_NOT_EQUAL(f1->signature(Node::SignatureReturnType),
                               f2->signature(Node::SignatureReturnType));
    }

    LT_RETURN_IF_NOT_EQUAL(n1->nodeType(), n2->nodeType());
    LT_RETURN_IF_NOT_EQUAL(n1->name(), n2->name());
    LT_RETURN_IF_NOT_EQUAL(n1->access(), n2->access());
    LT_RETURN_IF_NOT_EQUAL(n1->location().filePath(), n2->location().filePath());

    return false;
}

/*!
  \enum Node::NodeType

  An unsigned char value that identifies an object as a
  particular subclass of Node.

  \value NoType The node type has not been set yet.
  \value Namespace The Node subclass is NamespaceNode, which represents a C++
         namespace.
  \value Class The Node subclass is ClassNode, which represents a C++ class.
  \value Struct The Node subclass is ClassNode, which represents a C struct.
  \value Union The Node subclass is ClassNode, which represents a C union
         (currently no known uses).
  \value HeaderFile The Node subclass is HeaderNode, which represents a header
         file.
  \value Page The Node subclass is PageNode, which represents a text page from
         a .qdoc file.
  \value Enum The Node subclass is EnumNode, which represents an enum type or
         enum class.
  \value Example The Node subclass is ExampleNode, which represents an example
         subdirectory.
  \value ExternalPage The Node subclass is ExternalPageNode, which is for
         linking to an external page.
  \value Function The Node subclass is FunctionNode, which can represent C++,
         and QML functions.
  \value Typedef The Node subclass is TypedefNode, which represents a C++
         typedef.
  \value Property The Node subclass is PropertyNode, which represents a use of
         Q_Property.
  \value Variable The Node subclass is VariableNode, which represents a global
         variable or class data member.
  \value Group The Node subclass is CollectionNode, which represents a group of
         documents.
  \value Module The Node subclass is CollectionNode, which represents a C++
         module.
  \value QmlType The Node subclass is QmlTypeNode, which represents a QML type.
  \value QmlModule The Node subclass is CollectionNode, which represents a QML
         module.
  \value QmlProperty The Node subclass is QmlPropertyNode, which represents a
         property in a QML type.
  \value QmlBasicType The Node subclass is QmlTypeNode, which represents a
         value type like int, etc.
  \value SharedComment The Node subclass is SharedCommentNode, which represents
         a collection of nodes that share the same documentation comment.
  \omitvalue Collection
  \value Proxy The Node subclass is ProxyNode, which represents one or more
         entities that are documented in the current module but which actually
         reside in a different module.
  \omitvalue LastType
*/

/*!
  \enum Node::Genus

  An unsigned char value that specifies whether the Node represents a
  C++ element, a QML element, or a text document.
  The Genus values are also passed to search functions to specify the
  Genus of Tree Node that can satisfy the search.

  \value DontCare The Genus is not specified. Used when calling Tree search functions to indicate
                  the search can accept any Genus of Node.
  \value CPP The Node represents a C++ element.
  \value QML The Node represents a QML element.
  \value DOC The Node represents a text document.
*/

/*!
  \enum Access

  An unsigned char value that indicates the C++ access level.

  \value Public The element has public access.
  \value Protected The element has protected access.
  \value Private The element has private access.
*/

/*!
  \enum Node::Status

  An unsigned char that specifies the status of the documentation element in
  the documentation set.

  \value Deprecated The element has been deprecated.
  \value Preliminary The element is new; the documentation is preliminary.
  \value Active The element is current.
  \value Internal The element is for internal use only, not to be published.
  \value DontDocument The element is not to be documented.
*/

/*!
  \enum Node::ThreadSafeness

  An unsigned char that specifies the degree of thread-safeness of the element.

  \value UnspecifiedSafeness The thread-safeness is not specified.
  \value NonReentrant The element is not reentrant.
  \value Reentrant The element is reentrant.
  \value ThreadSafe The element is threadsafe.
*/

/*!
  \enum Node::LinkType

  An unsigned char value that probably should be moved out of the Node base class.

  \value StartLink
  \value NextLink
  \value PreviousLink
  \value ContentsLink
 */

/*!
  \enum Node::FlagValue

  A value used in PropertyNode and QmlPropertyNode that can be -1, 0, or +1.
  Properties and QML properties have flags, which can be 0 or 1, false or true,
  or not set. FlagValueDefault is the not set value. In other words, if a flag
  is set to FlagValueDefault, the meaning is the flag has not been set.

  \value FlagValueDefault -1 Not set.
  \value FlagValueFalse 0 False.
  \value FlagValueTrue 1 True.
*/

/*!
  \fn Node::~Node()

  The default destructor is virtual so any subclass of Node can be
  deleted by deleting a pointer to Node.
 */

/*! \fn bool Node::isActive() const
  Returns true if this node's status is \c Active.
 */

/*! \fn bool Node::isClass() const
  Returns true if the node type is \c Class.
 */

/*! \fn bool Node::isCppNode() const
  Returns true if this node's Genus value is \c CPP.
 */

/*! \fn bool Node::isDeprecated() const
  Returns true if this node's status is \c Deprecated.
 */

/*! \fn bool Node::isDontDocument() const
  Returns true if this node's status is \c DontDocument.
 */

/*! \fn bool Node::isEnumType() const
  Returns true if the node type is \c Enum.
 */

/*! \fn bool Node::isExample() const
  Returns true if the node type is \c Example.
 */

/*! \fn bool Node::isExternalPage() const
  Returns true if the node type is \c ExternalPage.
 */

/*! \fn bool Node::isFunction(Genus g = DontCare) const
  Returns true if this is a FunctionNode and its Genus is set to \a g.
 */

/*! \fn bool Node::isGroup() const
  Returns true if the node type is \c Group.
 */

/*! \fn bool Node::isHeader() const
  Returns true if the node type is \c HeaderFile.
 */

/*! \fn bool Node::isIndexNode() const
  Returns true if this node was created from something in an index file.
 */

/*! \fn bool Node::isModule() const
  Returns true if the node type is \c Module.
 */

/*! \fn bool Node::isNamespace() const
  Returns true if the node type is \c Namespace.
 */

/*! \fn bool Node::isPage() const
  Returns true if the node type is \c Page.
 */

/*! \fn bool Node::isPreliminary() const
  Returns true if this node's status is \c Preliminary.
 */

/*! \fn bool Node::isPrivate() const
  Returns true if this node's access is \c Private.
 */

/*! \fn bool Node::isProperty() const
  Returns true if the node type is \c Property.
 */

/*! \fn bool Node::isProxyNode() const
  Returns true if the node type is \c Proxy.
 */

/*! \fn bool Node::isPublic() const
  Returns true if this node's access is \c Public.
 */

/*! \fn bool Node::isProtected() const
  Returns true if this node's access is \c Protected.
 */

/*! \fn bool Node::isQmlBasicType() const
  Returns true if the node type is \c QmlBasicType.
 */

/*! \fn bool Node::isQmlModule() const
  Returns true if the node type is \c QmlModule.
 */

/*! \fn bool Node::isQmlNode() const
  Returns true if this node's Genus value is \c QML.
 */

/*! \fn bool Node::isQmlProperty() const
  Returns true if the node type is \c QmlProperty.
 */

/*! \fn bool Node::isQmlType() const
  Returns true if the node type is \c QmlType or \c QmlValueType.
 */

/*! \fn bool Node::isRelatedNonmember() const
  Returns true if this is a related nonmember of something.
 */

/*! \fn bool Node::isStruct() const
  Returns true if the node type is \c Struct.
 */

/*! \fn bool Node::isSharedCommentNode() const
  Returns true if the node type is \c SharedComment.
 */

/*! \fn bool Node::isTypeAlias() const
  Returns true if the node type is \c Typedef.
 */

/*! \fn bool Node::isTypedef() const
  Returns true if the node type is \c Typedef.
 */

/*! \fn bool Node::isUnion() const
  Returns true if the node type is \c Union.
 */

/*! \fn bool Node::isVariable() const
  Returns true if the node type is \c Variable.
 */

/*! \fn bool Node::isGenericCollection() const
  Returns true if the node type is \c Collection.
 */

/*! \fn bool Node::isAbstract() const
  Returns true if the ClassNode or QmlTypeNode is marked abstract.
*/

/*! \fn bool Node::isAggregate() const
  Returns true if this node is an aggregate, which means it
  inherits Aggregate and can therefore have children.
*/

/*! \fn bool Node::isFirstClassAggregate() const
  Returns true if this Node is an Aggregate but not a ProxyNode.
*/

/*! \fn bool Node::isAlias() const
  Returns true if this QML property is marked as an alias.
*/

/*! \fn bool Node::isAttached() const
  Returns true if the QML property or QML method node is marked as attached.
*/

/*! \fn bool Node::isClassNode() const
  Returns true if this is an instance of ClassNode.
*/

/*! \fn bool Node::isCollectionNode() const
  Returns true if this is an instance of CollectionNode.
*/

/*! \fn bool Node::isDefault() const
  Returns true if the QML property node is marked as default.
*/

/*! \fn bool Node::isMacro() const
  returns true if either FunctionNode::isMacroWithParams() or
  FunctionNode::isMacroWithoutParams() returns true.
*/

/*! \fn bool Node::isPageNode() const
  Returns true if this node represents something that generates a documentation
  page. In other words, if this Node's subclass inherits PageNode, then this
  function will return \e true.
*/

/*! \fn bool Node::isQtQuickNode() const
  Returns true if this node represents a QML element in the QtQuick module.
*/

/*! \fn bool Node::isRelatableType() const
  Returns true if this node is something you can relate things to with
  the \e relates command. NamespaceNode, ClassNode, HeaderNode, and
  ProxyNode are relatable types.
*/

/*! \fn bool Node::isMarkedReimp() const
  Returns true if the FunctionNode is marked as a reimplemented function.
  That means it is virtual in the base class and it is reimplemented in
  the subclass.
*/

/*! \fn bool Node::isPropertyGroup() const
  Returns true if the node is a SharedCommentNode for documenting
  multiple C++ properties or multiple QML properties.
*/

/*! \fn bool Node::isStatic() const
  Returns true if the FunctionNode represents a static function.
*/

/*! \fn bool Node::isTextPageNode() const
  Returns true if the node is a PageNode but not an Aggregate.
*/

/*!
  Returns this node's name member. Appends "()" to the returned
  name if this node is a function node, but not if it is a macro
  because macro names normally appear without parentheses.
 */
QString Node::plainName() const
{
    if (isFunction() && !isMacro())
        return m_name + QLatin1String("()");
    return m_name;
}

/*!
  Constructs and returns the node's fully qualified name by
  recursively ascending the parent links and prepending each
  parent name + "::". Breaks out when reaching a HeaderNode,
  or when the parent pointer is \a relative. Typically, calls
  to this function pass \c nullptr for \a relative.
 */
QString Node::plainFullName(const Node *relative) const
{
    if (m_name.isEmpty())
        return QLatin1String("global");
    if (isHeader())
        return plainName();

    QStringList parts;
    const Node *node = this;
    while (node && !node->isHeader()) {
        parts.prepend(node->plainName());
        if (node->parent() == relative || node->parent()->name().isEmpty())
            break;
        node = node->parent();
    }
    return parts.join(QLatin1String("::"));
}

/*!
  Constructs and returns the node's fully qualified signature
  by recursively ascending the parent links and prepending each
  parent name + "::" to the plain signature. The return type is
  not included.
 */
QString Node::plainSignature() const
{
    if (m_name.isEmpty())
        return QLatin1String("global");

    QString fullName;
    const Node *node = this;
    while (node) {
        fullName.prepend(node->signature(Node::SignaturePlain));
        if (node->parent()->name().isEmpty())
            break;
        fullName.prepend(QLatin1String("::"));
        node = node->parent();
    }
    return fullName;
}

/*!
  Constructs and returns this node's full name. The full name is
  often just the title(). When it is not the title, it is the
  plainFullName().
 */
QString Node::fullName(const Node *relative) const
{
    if ((isTextPageNode() || isGroup()) && !title().isEmpty())
        return title();
    return plainFullName(relative);
}

/*!
  Sets this Node's Doc to \a doc. If \a replace is false and
  this Node already has a Doc, and if this doc is not marked
  with the \\reimp command, a warning is reported that the
  existing Doc is being overridden, and it reports where the
  previous Doc was found. If \a replace is true, the Doc is
  replaced silently.
 */
void Node::setDoc(const Doc &doc, bool replace)
{
    if (!m_doc.isEmpty() && !replace && !doc.isMarkedReimp()) {
        doc.location().warning(QStringLiteral("Overrides a previous doc"),
                QStringLiteral("from here: %1").arg(m_doc.location().toString()));
    }
    m_doc = doc;
}

/*!
  Sets the node's status to \a t.

  \sa Status
*/
void Node::setStatus(Status t)
{
    m_status = t;

    // Set non-null, empty URL to nodes that are ignored as
    // link targets
    switch (t) {
    case Internal:
        if (Config::instance().showInternal())
            break;
        Q_FALLTHROUGH();
    case DontDocument:
        m_url = QStringLiteral("");
        break;
    default:
        break;
    }
}

/*!
  Construct a node with the given \a type and having the
  given \a parent and \a name. The new node is added to the
  parent's child list.
 */
Node::Node(NodeType type, Aggregate *parent, QString name)
    : m_nodeType(type),
      m_indexNodeFlag(false),
      m_relatedNonmember(false),
      m_hadDoc(false),
      m_parent(parent),
      m_name(std::move(name))
{
    if (m_parent)
        m_parent->addChild(this);

    m_outSubDir = Generator::outputSubdir();

    setGenus(getGenus(type));
}

/*!
  Determines the appropriate Genus value for the NodeType
  value \a t and returns that Genus value. Note that this
  function is called in the Node() constructor. It always
  returns Node::CPP when \a t is Node::Function, which
  means the FunctionNode() constructor must determine its
  own Genus value separately, because class FunctionNode
  is a subclass of Node.
 */
Node::Genus Node::getGenus(Node::NodeType t)
{
    switch (t) {
    case Node::Enum:
    case Node::Class:
    case Node::Struct:
    case Node::Union:
    case Node::Module:
    case Node::TypeAlias:
    case Node::Typedef:
    case Node::Property:
    case Node::Variable:
    case Node::Function:
    case Node::Namespace:
    case Node::HeaderFile:
        return Node::CPP;
    case Node::QmlType:
    case Node::QmlModule:
    case Node::QmlProperty:
    case Node::QmlValueType:
        return Node::QML;
    case Node::Page:
    case Node::Group:
    case Node::Example:
    case Node::ExternalPage:
        return Node::DOC;
    case Node::Collection:
    case Node::SharedComment:
    case Node::Proxy:
    default:
        return Node::DontCare;
    }
}

/*! \fn QString Node::url() const
  Returns the node's URL, which is the url of the documentation page
  created for the node or the url of an external page if the node is
  an ExternalPageNode. The url is used for generating a link to the
  page the node represents.

  \sa Node::setUrl()
 */

/*! \fn void Node::setUrl(const QString &url)
  Sets the node's URL to \a url, which is the url to the page that the
  node represents. This function is only called when an index file is
  read. In other words, a node's url is set when qdoc decides where its
  page will be and what its name will be, which happens when qdoc writes
  the index file for the module being documented.

  \sa QDocIndexFiles
 */

/*!
  Returns this node's type as a string for use as an
  attribute value in XML or HTML.
 */
QString Node::nodeTypeString() const
{
    if (isFunction()) {
        const auto *fn = static_cast<const FunctionNode *>(this);
        return fn->kindString();
    }
    return nodeTypeString(nodeType());
}

/*!
  Returns the node type \a t as a string for use as an
  attribute value in XML or HTML.
 */
QString Node::nodeTypeString(NodeType t)
{
    switch (t) {
    case Namespace:
        return QLatin1String("namespace");
    case Class:
        return QLatin1String("class");
    case Struct:
        return QLatin1String("struct");
    case Union:
        return QLatin1String("union");
    case HeaderFile:
        return QLatin1String("header");
    case Page:
        return QLatin1String("page");
    case Enum:
        return QLatin1String("enum");
    case Example:
        return QLatin1String("example");
    case ExternalPage:
        return QLatin1String("external page");
    case TypeAlias:
    case Typedef:
        return QLatin1String("typedef");
    case Function:
        return QLatin1String("function");
    case Property:
        return QLatin1String("property");
    case Proxy:
        return QLatin1String("proxy");
    case Variable:
        return QLatin1String("variable");
    case Group:
        return QLatin1String("group");
    case Module:
        return QLatin1String("module");

    case QmlType:
        return QLatin1String("QML type");
    case QmlValueType:
        return QLatin1String("QML value type");
    case QmlModule:
        return QLatin1String("QML module");
    case QmlProperty:
        return QLatin1String("QML property");

    case SharedComment:
        return QLatin1String("shared comment");
    case Collection:
        return QLatin1String("collection");
    default:
        break;
    }
    return QString();
}

/*! Converts the boolean value \a b to an enum representation
  of the boolean type, which includes an enum value for the
  \e {default value} of the item, i.e. true, false, or default.
 */
Node::FlagValue Node::toFlagValue(bool b)
{
    return b ? FlagValueTrue : FlagValueFalse;
}

/*!
  Converts the enum \a fv back to a boolean value.
  If \a fv is neither the true enum value nor the
  false enum value, the boolean value returned is
  \a defaultValue.
 */
bool Node::fromFlagValue(FlagValue fv, bool defaultValue)
{
    switch (fv) {
    case FlagValueTrue:
        return true;
    case FlagValueFalse:
        return false;
    default:
        return defaultValue;
    }
}

/*!
  This function creates a pair that describes a link.
  The pair is composed from \a link and \a desc. The
  \a linkType is the map index the pair is filed under.
 */
void Node::setLink(LinkType linkType, const QString &link, const QString &desc)
{
    std::pair<QString, QString> linkPair;
    linkPair.first = link;
    linkPair.second = desc;
    m_linkMap[linkType] = linkPair;
}

/*!
    Sets the information about the project and version a node was introduced
    in, unless the version is lower than the 'ignoresince.<project>'
    configuration variable.
 */
void Node::setSince(const QString &since)
{
    QStringList parts = since.split(QLatin1Char(' '));
    QString project;
    if (parts.size() > 1)
        project = Config::dot + parts.first();

    QVersionNumber cutoff =
            QVersionNumber::fromString(Config::instance().get(CONFIG_IGNORESINCE + project).asString())
                    .normalized();

    if (!cutoff.isNull() && QVersionNumber::fromString(parts.last()).normalized() < cutoff)
        return;

    m_since = parts.join(QLatin1Char(' '));
}

/*!
  Extract a class name from the type \a string and return it.
 */
QString Node::extractClassName(const QString &string) const
{
    QString result;
    for (int i = 0; i <= string.size(); ++i) {
        QChar ch;
        if (i != string.size())
            ch = string.at(i);

        QChar lower = ch.toLower();
        if ((lower >= QLatin1Char('a') && lower <= QLatin1Char('z')) || ch.digitValue() >= 0
            || ch == QLatin1Char('_') || ch == QLatin1Char(':')) {
            result += ch;
        } else if (!result.isEmpty()) {
            if (result != QLatin1String("const"))
                return result;
            result.clear();
        }
    }
    return result;
}

/*!
  Returns the thread safeness value for whatever this node
  represents. But if this node has a parent and the thread
  safeness value of the parent is the same as the thread
  safeness value of this node, what is returned is the
  value \c{UnspecifiedSafeness}. Why?
 */
Node::ThreadSafeness Node::threadSafeness() const
{
    if (m_parent && m_safeness == m_parent->inheritedThreadSafeness())
        return UnspecifiedSafeness;
    return m_safeness;
}

/*!
  If this node has a parent, the parent's thread safeness
  value is returned. Otherwise, this node's thread safeness
  value is returned. Why?
 */
Node::ThreadSafeness Node::inheritedThreadSafeness() const
{
    if (m_parent && m_safeness == UnspecifiedSafeness)
        return m_parent->inheritedThreadSafeness();
    return m_safeness;
}

/*!
  Returns \c true if the node's status is \c Internal, or if
  its parent is a class with \c Internal status.
 */
bool Node::isInternal() const
{
    if (status() == Internal)
        return true;
    return parent() && parent()->status() == Internal && !parent()->isAbstract();
}

/*! \fn void Node::markInternal()
  Sets the node's access to Private and its status to Internal.
 */

/*!
  Returns a pointer to the root of the Tree this node is in.
 */
Aggregate *Node::root() const
{
    if (parent() == nullptr)
        return (this->isAggregate() ? static_cast<Aggregate *>(const_cast<Node *>(this)) : nullptr);
    Aggregate *t = parent();
    while (t->parent() != nullptr)
        t = t->parent();
    return t;
}

/*!
  Returns a pointer to the Tree this node is in.
 */
Tree *Node::tree() const
{
    return root()->tree();
}

/*!
  Sets the node's declaration location, its definition
  location, or both, depending on the suffix of the file
  name from the file path in location \a t.
 */
void Node::setLocation(const Location &t)
{
    QString suffix = t.fileSuffix();
    if (suffix == "h")
        m_declLocation = t;
    else if (suffix == "cpp")
        m_defLocation = t;
    else {
        m_declLocation = t;
        m_defLocation = t;
    }
}

/*!
  Returns true if this node is sharing a comment and the
  shared comment is not empty.
 */
bool Node::hasSharedDoc() const
{
    return (m_sharedCommentNode && m_sharedCommentNode->hasDoc());
}

/*!
  Returns the CPP node's qualified name by prepending the
  namespaces name + "::" if there isw a namespace.
 */
QString Node::qualifyCppName()
{
    if (m_parent && m_parent->isNamespace() && !m_parent->name().isEmpty())
        return m_parent->name() + "::" + m_name;
    return m_name;
}

/*!
  Return the name of this node qualified with the parent name
  and "::" if there is a parent name.
 */
QString Node::qualifyWithParentName()
{
    if (m_parent && !m_parent->name().isEmpty())
        return m_parent->name() + "::" + m_name;
    return m_name;
}

/*!
  Returns the QML node's qualified name by prepending the logical
  module name.
 */
QString Node::qualifyQmlName()
{
    return logicalModuleName() + "::" + m_name;
}

/*!
  Returns \c true if the node is a class node or a QML type node
  that is marked as being a wrapper class or wrapper QML type,
  or if it is a member of a wrapper class or type.
 */
bool Node::isWrapper() const
{
    return m_parent != nullptr && m_parent->isWrapper();
}

/*!
  Construct the full document name for this node and return it.
 */
QString Node::fullDocumentName() const
{
    QStringList pieces;
    const Node *n = this;

    do {
        if (!n->name().isEmpty())
            pieces.insert(0, n->name());

        if (n->isQmlType() && !n->logicalModuleName().isEmpty()) {
            pieces.insert(0, n->logicalModuleName());
            break;
        }

        if (n->isTextPageNode())
            break;

        // Examine the parent if the node is a member
        if (!n->parent() || n->isRelatedNonmember())
            break;

        n = n->parent();
    } while (true);

    // Create a name based on the type of the ancestor node.
    QString concatenator = "::";
    if (n->isQmlType())
        concatenator = QLatin1Char('.');

    if (n->isTextPageNode())
        concatenator = QLatin1Char('#');

    return pieces.join(concatenator);
}

void Node::setDeprecatedSince(const QString &sinceVersion)
{
    if (!m_deprecatedSince.isEmpty())
        qCWarning(lcQdoc) << QStringLiteral(
                                     "Setting deprecated since version for %1 to %2 even though it "
                                     "was already set to %3. This is very unexpected.")
                                     .arg(this->m_name, sinceVersion, this->m_deprecatedSince);
    m_deprecatedSince = sinceVersion;
}

/*! \fn Node *Node::clone(Aggregate *parent)

  When reimplemented in a subclass, this function creates a
  clone of this node on the heap and makes the clone a child
  of \a parent. A pointer to the clone is returned.

  Here in the base class, this function does nothing and returns
  nullptr.
 */

/*! \fn NodeType Node::nodeType() const
  Returns this node's type.

  \sa NodeType
*/

/*! \fn Genus Node::genus() const
  Returns this node's Genus.

  \sa Genus
*/

/*! void Node::setGenus(Genus t)
  Sets this node's Genus to \a t.
*/

/*! \fn  QString Node::signature(Node::SignatureOptions options) const

  Specific parts of the signature are included according to flags in
  \a options.

  If this node is not a FunctionNode, this function returns plainName().

  \sa FunctionNode::signature()
*/

/*! \fn const QString &Node::fileNameBase() const
  Returns the node's file name base string, which is built once, when
  Generator::fileBase() is called and stored in the Node.
*/

/*! \fn bool Node::hasFileNameBase() const
  Returns true if the node's file name base has been set.

  \sa Node::fileNameBase()
*/

/*! \fn void Node::setFileNameBase(const QString &t)
  Sets the node's file name base to \a t. Only called by
  Generator::fileBase().
*/

/*! \fn void Node::setAccess(Access t)
  Sets the node's access type to \a t.

  \sa Access
*/

/*! \fn void Node::setThreadSafeness(ThreadSafeness t)
  Sets the node's thread safeness to \a t.

  \sa ThreadSafeness
*/

/*! \fn void Node::setPhysicalModuleName(const QString &name)
  Sets the node's physical module \a name.
*/

/*! \fn void Node::setReconstitutedBrief(const QString &t)
  When reading an index file, this function is called with the
  reconstituted brief clause \a t to set the node's brief clause.
  I think this is needed for linking to something in the brief clause.
*/

/*! \fn void Node::setParent(Aggregate *n)
  Sets the node's parent pointer to \a n. Such a thing
  is not lightly done. All the calls to this function
  are in other member functions of Node subclasses. See
  the code in the subclass implementations to understand
  when this function can be called safely and why it is called.
*/

/*! \fn void Node::setIndexNodeFlag(bool isIndexNode = true)
  Sets a flag in this Node that indicates the node was created
  for something in an index file. This is important to know
  because an index node is not to be documented in the current
  module. When the index flag is set, it means the Node
  represents something in another module, and it will be
  documented in that module's documentation.
*/

/*! \fn void Node::setRelatedNonmember(bool b)
  Sets a flag in the node indicating whether this node is a related nonmember
  of something. This function is called when the \c relates command is seen.
 */

/*! \fn void Node::addMember(Node *node)
  In a CollectionNode, this function adds \a node to the collection
  node's members list. It does nothing if this node is not a CollectionNode.
 */

/*! \fn bool Node::hasNamespaces() const
  Returns \c true if this is a CollectionNode and its members list
  contains namespace nodes. Otherwise it returns \c false.
 */

/*! \fn bool Node::hasClasses() const
  Returns \c true if this is a CollectionNode and its members list
  contains class nodes. Otherwise it returns \c false.
 */

/*! \fn void Node::setAbstract(bool b)
  If this node is a ClassNode or a QmlTypeNode, the node's abstract flag
  data member is set to \a b.
 */

/*! \fn void Node::setWrapper()
  If this node is a ClassNode or a QmlTypeNode, the node's wrapper flag
  data member is set to \c true.
 */

/*! \fn void Node::setDataType(const QString &dataType)
  If this node is a PropertyNode or a QmlPropertyNode, its
  data type data member is set to \a dataType. Otherwise,
  this function does nothing.
 */

/*! \fn bool Node::wasSeen() const
  Returns the \c seen flag data member of this node if it is a NamespaceNode
  or a CollectionNode. Otherwise it returns \c false. If \c true is returned,
  it means that the location where the namespace or collection is to be
  documented has been found.
 */

/*! \fn void appendGroupName(const QString &t)
  If this node is a PageNode, the group name \a t is appended to the node's
  list of group names. It is not clear to me what this list of group names
  is used for, but it is written to the index file, and it is used in the
  navigation bar.
 */

/*! \fn QString Node::element() const
  If this node is a QmlPropertyNode or a FunctionNode, this function
  returns the name of the parent node. Otherwise it returns an empty
  string.
 */

/*! \fn bool Node::docMustBeGenerated() const
  This function is called to perform a test to decide if the node must have
  documentation generated. In the Node base class, it always returns \c false.

  In the ProxyNode class it always returns \c true. There aren't many proxy
  nodes, but when one appears, it must generate documentation. In the overrides
  in NamespaceNode and ClassNode, a meaningful test is performed to decide if
  documentation must be generated.
 */

/*! \fn QString Node::title() const
  Returns a string that can be used to print a title in the documentation for
  whatever this Node is. In the Node base class, the node's name() is returned.
  In a PageNode, the function returns the title data member. In a HeaderNode,
  if the title() is empty, the name() is returned.
 */

/*! \fn QString Node::subtitle() const { return QString(); }
  Returns a string that can be used to print a subtitle in the documentation for
  whatever this Node is. In the Node base class, the empty string is returned.
  In a PageNode, the function returns the subtitle data member. In a HeaderNode,
  the subtitle data member is returned.
 */

/*! \fn QString Node::fullTitle() const
  Returns a string that can be used as the full title for the documentation of
  this node. In this base class, the name() is returned. In a PageNode, title()
  is returned. In a HeaderNode, if the title() is empty, the name() is returned.
  If the title() is not empty then name-title is returned. In a CollectionNode,
  the title() is returned.
 */

/*! \fn bool Node::setTitle(const QString &title)
  Sets the node's \a title, which is used for the title of
  the documentation page, if one is generated for this node.
  Returns \c true if the title is set. In this base class,
  there is no title string stored, so in the base class,
  nothing happens and \c false is returned. The override in
  the PageNode class is where the title is set.
 */

/*! \fn bool Node::setSubtitle(const QString &subtitle)
  Sets the node's \a subtitle, which is used for the subtitle
  of the documentation page, if one is generated for this node.
  Returns \c true if the subtitle is set. In this base class,
  there is no subtitle string stored, so in the base class,
  nothing happens and \c false is returned. The override in
  the PageNode and HeaderNode classes is where the subtitle is
  set.
 */

/*! \fn void Node::markDefault()
  If this node is a QmlPropertyNode, it is marked as the default property.
  Otherwise the function does nothing.
 */

/*! \fn void Node::markReadOnly(bool flag)
  If this node is a QmlPropertyNode, then the property's read-only
  flag is set to \a flag.
 */

/*! \fn Aggregate *Node::parent() const
  Returns the node's parent pointer.
*/

/*! \fn const QString &Node::name() const
  Returns the node's name data member.
*/

/*! \fn void Node::setQtVariable(const QString &v)
  If this node is a CollectionNode, its QT variable is set to \a v.
  Otherwise the function does nothing. I don't know what the QT variable
  is used for.
 */

/*! \fn QString Node::qtVariable() const
  If this node is a CollectionNode, its QT variable is returned.
  Otherwise an empty string is returned. I don't know what the QT
  variable is used for.
 */

/*! \fn bool Node::hasTag(const QString &t) const
  If this node is a FunctionNode, the function returns \c true if
  the function has the tag \a t. Otherwise the function returns
  \c false. I don't know what the tag is used for.
 */

/*! \fn const QMap<LinkType, std::pair<QString, QString> > &Node::links() const
  Returns a reference to this node's link map. The link map should
  probably be moved to the PageNode, because it contains links to the
  start page, next page, previous page, and contents page, and these
  are only used in PageNode, I think.
 */

/*! \fn Access Node::access() const
  Returns the node's Access setting, which can be \c Public,
  \c Protected, or \c Private.
 */

/*! \fn const Location& Node::declLocation() const
  Returns the Location where this node's declaration was seen.
  Normally the declaration location is in an \e include file.
  The declaration location is used in qdoc error/warning messages
  about the declaration.
 */

/*! \fn const Location& Node::defLocation() const
  Returns the Location where this node's dedefinition was seen.
  Normally the definition location is in a \e .cpp file.
  The definition location is used in qdoc error/warning messages
  when the error is discovered at the location of the definition,
  although the way to correct the problem often requires changing
  the declaration.
 */

/*! \fn const Location& Node::location() const
  If this node's definition location is empty, this function
  returns this node's declaration location. Otherwise it
  returns the definition location.

  \sa Location
 */

/*! \fn const Doc &Node::doc() const
  Returns a reference to the node's Doc data member.

  \sa Doc
 */

/*! \fn bool Node::hasDoc() const
  Returns \c true if the node has documentation, i.e. if its Doc
  data member is not empty.

  \sa Doc
 */

/*! \fn Status Node::status() const
  Returns the node's status value.

  \sa Status
 */

/*! \fn QString Node::since() const
  Returns the node's since string, which can be empty.
 */

/*! \fn QString Node::templateStuff() const
  Returns the node's template parameters string, if this node
  represents a templated element.
 */

/*! \fn bool Node::isSharingComment() const
  This function returns \c true if the node is sharing a comment
  with other nodes. For example, multiple functions can be documented
  with a single qdoc comment by listing the \c {\\fn} signatures for
  all the functions in the single qdoc comment.
 */

/*! \fn QString Node::qmlTypeName() const
  If this is a QmlPropertyNode or a FunctionNode representing a QML
  method, this function returns the qmlTypeName() of
  the parent() node. Otherwise it returns the name data member.
 */

/*! \fn QString Node::qmlFullBaseName() const
  If this is a QmlTypeNode, this function returns the QML full
  base name. Otherwise it returns an empty string.
 */

/*! \fn QString Node::logicalModuleName() const
  If this is a CollectionNode, this function returns the logical
  module name. Otherwise it returns an empty string.
 */

/*! \fn QString Node::logicalModuleVersion() const
  If this is a CollectionNode, this function returns the logical
  module version number. Otherwise it returns an empty string.
 */

/*! \fn QString Node::logicalModuleIdentifier() const
  If this is a CollectionNode, this function returns the logical
  module identifier. Otherwise it returns an empty string.
 */

/*! \fn void Node::setLogicalModuleInfo(const QString &arg)
  If this node is a CollectionNode, this function splits \a arg
  on the blank character to get a logical module name and version
  number. If the version number is present, it splits the version
  number on the '.' character to get a major version number and a
  minor version number. If the version number is present, both the
  major and minor version numbers should be there, but the minor
  version number is not absolutely necessary.

  The strings are stored in the appropriate data members for use
  when the QML module page is generated.
 */

/*! \fn void Node::setLogicalModuleInfo(const QStringList &info)
  If this node is a CollectionNode, this function accepts the
  logical module \a info as a string list. If the logical module
  info contains the version number, it splits the version number
  on the '.' character to get the major and minor version numbers.
  Both major and minor version numbers should be provided, but
  the minor version number is not strictly necessary.

  The strings are stored in the appropriate data members for use
  when the QML module page is generated. This overload
  of the function is called when qdoc is reading an index file.
 */

/*! \fn CollectionNode *Node::logicalModule() const
  If this is a QmlTypeNode, a pointer to its QML module is returned,
  which is a pointer to a CollectionNode. Otherwise the \c nullptr
  is returned.
 */

/*! \fn void Node::setQmlModule(CollectionNode *t)
  If this is a QmlTypeNode, this function sets the QML type's QML module
  pointer to the CollectionNode \a t. Otherwise the function does nothing.
 */

/*! \fn ClassNode *Node::classNode()
  If this is a QmlTypeNode, this function returns the pointer to
  the C++ ClassNode that this QML type represents. Otherwise the
  \c nullptr is returned.
 */

/*! \fn void Node::setClassNode(ClassNode *cn)
  If this is a QmlTypeNode, this function sets the C++ class node
  to \a cn. The C++ ClassNode is the C++ implementation of the QML
  type.
 */

/*! \fn const QString &Node::outputSubdirectory() const
  Returns the node's output subdirector, which is the subdirectory
  of the output directory where the node's documentation file is
  written.
 */

/*! \fn void Node::setOutputSubdirectory(const QString &t)
  Sets the node's output subdirectory to \a t. This is the subdirector
  of the output directory where the node's documentation file will be
  written.
 */

/*! \fn NodeType Node::goal(const QString &t)
  When a square-bracket parameter is used in a qdoc command, this
  function might be called to convert the text string \a t obtained
  from inside the square brackets to be a Goal value, which is returned.

  \sa Goal
 */

QT_END_NAMESPACE
