/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "node.h"
#include "tree.h"
#include "codemarker.h"
#include "cppcodeparser.h"
#include <quuid.h>
#include "qdocdatabase.h"
#include <qdebug.h>
#include "generator.h"
#include "tokenizer.h"
#include "puredocparser.h"

QT_BEGIN_NAMESPACE

int Node::propertyGroupCount_ = 0;
QStringMap Node::operators_;
QMap<QString,Node::NodeType> Node::goals_;

/*!
  Initialize the map of search goals. This is called once
  by QDocDatabase::initializeDB(). The map key is a string
  representing a value in the enum Node::NodeType. The map value
  is the enum value.

  There should be an entry in the map for each value in the
  NodeType enum.
 */
void Node::initialize()
{
    goals_.insert("namespace", Node::Namespace);
    goals_.insert("class", Node::Class);
    goals_.insert("struct", Node::Struct);
    goals_.insert("union", Node::Union);
    goals_.insert("header", Node::HeaderFile);
    goals_.insert("headerfile", Node::HeaderFile);
    goals_.insert("page", Node::Page);
    goals_.insert("enum", Node::Enum);
    goals_.insert("example", Node::Example);
    goals_.insert("externalpage", Node::ExternalPage);
    goals_.insert("typedef", Node::Typedef);
    goals_.insert("typealias", Node::Typedef);
    goals_.insert("function", Node::Function);
    goals_.insert("proxy", Node::Proxy);
    goals_.insert("property", Node::Property);
    goals_.insert("variable", Node::Variable);
    goals_.insert("group", Node::Group);
    goals_.insert("module", Node::Module);
    goals_.insert("qmltype", Node::QmlType);
    goals_.insert("qmlmodule", Node::QmlModule);
    goals_.insert("qmlproperty", Node::QmlProperty);
    goals_.insert("qmlsignal", Node::Function);
    goals_.insert("qmlsignalhandler", Node::Function);
    goals_.insert("qmlmethod", Node::Function);
    goals_.insert("qmlbasictype", Node::QmlBasicType);
    goals_.insert("sharedcomment", Node::SharedComment);
    goals_.insert("collection", Node::Collection);
}

/*!
  If this Node's type is \a from, change the type to \a to
  and return \c true. Otherwise return false. This function
  is used to change Qml node types to Javascript node types,
  because these nodes are created as Qml nodes before it is
  discovered that the entity represented by the node is not
  Qml but javascript.

  Note that if the function returns true, which means the node
  type was indeed changed, then the node's Genus is also changed
  from QML to JS.

  The function also works in the other direction, but there is
  no use case for that.
 */
bool Node::changeType(NodeType from, NodeType to)
{
    if (nodeType_ == from) {
        nodeType_ = to;
        switch (to) {
          case QmlType:
          case QmlModule:
          case QmlProperty:
          case QmlBasicType:
              setGenus(Node::QML);
              break;
          case JsType:
          case JsModule:
          case JsProperty:
          case JsBasicType:
              setGenus(Node::JS);
              break;
          default:
              setGenus(Node::CPP);
              break;
        }
        return true;
    }
    return false;
}

/*!
  Returns \c true if the node \a n1 is less than node \a n2. The
  comparison is performed by comparing properties of the nodes
  in order of increasing complexity.
 */
bool Node::nodeNameLessThan(const Node *n1, const Node *n2)
{
    if (n1->isPageNode() && n2->isPageNode()) {
        const PageNode* f1 = static_cast<const PageNode*>(n1);
        const PageNode* f2 = static_cast<const PageNode*>(n2);
        if (f1->fullTitle() < f2->fullTitle())
            return true;
        else if (f1->fullTitle() > f2->fullTitle())
            return false;
    }

    if (n1->isFunction() && n2->isFunction()) {
        const FunctionNode* f1 = static_cast<const FunctionNode*>(n1);
        const FunctionNode* f2 = static_cast<const FunctionNode*>(n2);

        if (f1->isConst() < f2->isConst())
            return true;
        else if (f1->isConst() > f2->isConst())
            return false;

        if (f1->signature(false, false) < f2->signature(false, false))
            return true;
        else if (f1->signature(false, false) > f2->signature(false, false))
            return false;
    }

    if (n1->location().filePath() < n2->location().filePath())
        return true;
    else if (n1->location().filePath() > n2->location().filePath())
        return false;

    if (n1->nodeType() < n2->nodeType())
        return true;
    else if (n1->nodeType() > n2->nodeType())
        return false;

    if (n1->name() < n2->name())
        return true;
    else if (n1->name() > n2->name())
        return false;

    if (n1->access() < n2->access())
        return true;
    else if (n1->access() > n2->access())
        return false;

    return false;
}

/*!
  \class Node
  \brief The Node class is a node in the Tree.

  A Node represents a class or function or something else
  from the source code..
 */

/*!
  The destructor does nothing.
 */
Node::~Node()
{
    // nothing.
}

/*!
  Returns this node's name member. Appends "()" to the returned
  name if this node is a function node, but not if it is a macro
  because macro names normally appear without parentheses.
 */
QString Node::plainName() const
{
    if (isFunction() && !isMacro())
        return name_ + QLatin1String("()");
    return name_;
}

/*!
  Constructs and returns the node's fully qualified name by
  recursively ascending the parent links and prepending each
  parent name + "::". Breaks out when the parent pointer is
  \a relative. Almost all calls to this function pass 0 for
  \a relative.
 */
QString Node::plainFullName(const Node* relative) const
{
    if (name_.isEmpty())
        return QLatin1String("global");

    QString fullName;
    const Node* node = this;
    while (node) {
        fullName.prepend(node->plainName());
        if (node->parent() == relative || node->parent()->name().isEmpty())
            break;
        fullName.prepend(QLatin1String("::"));
        node = node->parent();
    }
    return fullName;
}

/*!
  Constructs and returns the node's fully qualified signature
  by recursively ascending the parent links and prepending each
  parent name + "::" to the plain signature. The return type is
  not included.
 */
QString Node::plainSignature() const
{
    if (name_.isEmpty())
        return QLatin1String("global");

    QString fullName;
    const Node* node = this;
    while (node) {
        fullName.prepend(node->signature(false, true));
        if (node->parent()->name().isEmpty())
            break;
        fullName.prepend(QLatin1String("::"));
        node = node->parent();
    }
    return fullName;
}

/*!
  Constructs and returns this node's full name.
 */
QString Node::fullName(const Node* relative) const
{
    if ((isTextPageNode() || isGroup()) && !title().isEmpty())
        return title();
    return plainFullName(relative);
}

/*!
  Try to match this node's type with one of the \a types.
  If a match is found, return true. If no match is found,
  return false.
 */
bool Node::match(const QList<int>& types) const
{
    for (int i=0; i<types.size(); ++i) {
        if (nodeType() == types.at(i))
            return true;
    }
    return false;
}

/*!
  Sets this Node's Doc to \a doc. If \a replace is false and
  this Node already has a Doc, and if this doc is not marked
  with the \\reimp command, a warning is reported that the
  existing Doc is being overridden, and it reports where the
  previous Doc was found. If \a replace is true, the Doc is
  replaced silently.
 */
void Node::setDoc(const Doc& doc, bool replace)
{
    if (!doc_.isEmpty() && !replace && !doc.isMarkedReimp()) {
        doc.location().warning(tr("Overrides a previous doc"));
        doc_.location().warning(tr("(The previous doc is here)"));
    }
    doc_ = doc;
}

/*!
  Construct a node with the given \a type and having the
  given \a parent and \a name. The new node is added to the
  parent's child list.
 */
Node::Node(NodeType type, Aggregate *parent, const QString& name)
    : nodeType_(type),
      access_(Public),
      safeness_(UnspecifiedSafeness),
      pageType_(NoPageType),
      status_(Active),
      indexNodeFlag_(false),
      relatedNonmember_(false),
      hadDoc_(false),
      parent_(parent),
      sharedCommentNode_(nullptr),
      name_(name)
{
    if (parent_)
        parent_->addChild(this);
    outSubDir_ = Generator::outputSubdir();
    if (operators_.isEmpty()) {
        operators_.insert("++","inc");
        operators_.insert("--","dec");
        operators_.insert("==","eq");
        operators_.insert("!=","ne");
        operators_.insert("<<","lt-lt");
        operators_.insert(">>","gt-gt");
        operators_.insert("+=","plus-assign");
        operators_.insert("-=","minus-assign");
        operators_.insert("*=","mult-assign");
        operators_.insert("/=","div-assign");
        operators_.insert("%=","mod-assign");
        operators_.insert("&=","bitwise-and-assign");
        operators_.insert("|=","bitwise-or-assign");
        operators_.insert("^=","bitwise-xor-assign");
        operators_.insert("<<=","bitwise-left-shift-assign");
        operators_.insert(">>=","bitwise-right-shift-assign");
        operators_.insert("||","logical-or");
        operators_.insert("&&","logical-and");
        operators_.insert("()","call");
        operators_.insert("[]","subscript");
        operators_.insert("->","pointer");
        operators_.insert("->*","pointer-star");
        operators_.insert("+","plus");
        operators_.insert("-","minus");
        operators_.insert("*","mult");
        operators_.insert("/","div");
        operators_.insert("%","mod");
        operators_.insert("|","bitwise-or");
        operators_.insert("&","bitwise-and");
        operators_.insert("^","bitwise-xor");
        operators_.insert("!","not");
        operators_.insert("~","bitwise-not");
        operators_.insert("<=","lt-eq");
        operators_.insert(">=","gt-eq");
        operators_.insert("<","lt");
        operators_.insert(">","gt");
        operators_.insert("=","assign");
        operators_.insert(",","comma");
        operators_.insert("delete[]","delete-array");
        operators_.insert("delete","delete");
        operators_.insert("new[]","new-array");
        operators_.insert("new","new");
    }
    setPageType(getPageType(type));
    setGenus(getGenus(type));
}

/*!
  Determines the appropriate PageType value for the NodeType
  value \a t and returns that PageType value.
 */
Node::PageType Node::getPageType(Node::NodeType t)
{
    switch (t) {
      case Node::Namespace:
      case Node::Class:
      case Node::Struct:
      case Node::Union:
      case Node::HeaderFile:
      case Node::Enum:
      case Node::Function:
      case Node::Typedef:
      case Node::Property:
      case Node::Variable:
      case Node::QmlType:
      case Node::QmlProperty:
      case Node::QmlBasicType:
      case Node::JsType:
      case Node::JsProperty:
      case Node::JsBasicType:
      case Node::SharedComment:
          return Node::ApiPage;
      case Node::Example:
          return Node::ExamplePage;
      case Node::Page:
      case Node::ExternalPage:
          return Node::NoPageType;
      case Node::Group:
      case Node::Module:
      case Node::QmlModule:
      case Node::JsModule:
      case Node::Collection:
          return Node::OverviewPage;
      case Node::Proxy:
      default:
          return Node::NoPageType;
    }
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
      case Node::QmlBasicType:
          return Node::QML;
      case Node::JsType:
      case Node::JsModule:
      case Node::JsProperty:
      case Node::JsBasicType:
          return Node::JS;
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
  Returns the node's URL.
 */

/*! \fn void Node::setUrl(const QString &url)
  Sets the node's URL to \a url
 */

/*!
  Returns this node's page type as a string, for use as an
  attribute value in XML or HTML.
 */
QString Node::pageTypeString() const
{
    return pageTypeString(pageType_);
}

/*!
  Returns the page type \a t as a string, for use as an
  attribute value in XML or HTML.
 */
QString Node::pageTypeString(PageType t)
{
    switch (t) {
    case Node::AttributionPage:
        return QLatin1String("attribution");
    case Node::ApiPage:
        return QLatin1String("api");
    case Node::ArticlePage:
        return QLatin1String("article");
    case Node::ExamplePage:
        return QLatin1String("example");
    case Node::HowToPage:
        return QLatin1String("howto");
    case Node::OverviewPage:
        return QLatin1String("overview");
    case Node::TutorialPage:
        return QLatin1String("tutorial");
    case Node::FAQPage:
        return QLatin1String("faq");
    default:
        return QLatin1String("article");
    }
}

/*!
  Returns this node's type as a string for use as an
  attribute value in XML or HTML.
 */
QString Node::nodeTypeString() const
{
    if (isFunction()) {
        const FunctionNode *fn = static_cast<const FunctionNode*>(this);
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
    case QmlBasicType:
        return QLatin1String("QML basic type");
    case QmlModule:
        return QLatin1String("QML module");
    case QmlProperty:
        return QLatin1String("QML property");

    case JsType:
        return QLatin1String("JS type");
    case JsBasicType:
        return QLatin1String("JS basic type");
    case JsModule:
        return QLatin1String("JS module");
    case JsProperty:
        return QLatin1String("JS property");

    case SharedComment:
        return QLatin1String("shared comment");
    case Collection:
        return QLatin1String("collection");
    default:
        break;
    }
    return QString();
}

/*!
  Set the page type according to the string \a t.
 */
void Node::setPageType(const QString& t)
{
    if ((t == "API") || (t == "api"))
        pageType_ = ApiPage;
    else if (t == "howto")
        pageType_ = HowToPage;
    else if (t == "overview")
        pageType_ = OverviewPage;
    else if (t == "tutorial")
        pageType_ = TutorialPage;
    else if (t == "faq")
        pageType_ = FAQPage;
    else if (t == "article")
        pageType_ = ArticlePage;
    else if (t == "example")
        pageType_ = ExamplePage;
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

  Note that runtimeDesignabilityFunction() should be called
  first. If that function returns the name of a function, it
  means the function must be called at runtime to determine
  whether the property is Designable.
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
    QPair<QString,QString> linkPair;
    linkPair.first = link;
    linkPair.second = desc;
    linkMap_[linkType] = linkPair;
}

/*!
    Sets the information about the project and version a node was introduced
    in. The string is simplified, removing excess whitespace before being
    stored.
 */
void Node::setSince(const QString &since)
{
    since_ = since.simplified();
}

/*!
  Returns a string representing the access specifier.
 */
QString Node::accessString() const
{
    switch (access_) {
    case Protected:
        return QLatin1String("protected");
    case Private:
        return QLatin1String("private");
    case Public:
    default:
        break;
    }
    return QLatin1String("public");
}

/*!
  Extract a class name from the type \a string and return it.
 */
QString Node::extractClassName(const QString &string) const
{
    QString result;
    for (int i=0; i<=string.size(); ++i) {
        QChar ch;
        if (i != string.size())
            ch = string.at(i);

        QChar lower = ch.toLower();
        if ((lower >= QLatin1Char('a') && lower <= QLatin1Char('z')) ||
                ch.digitValue() >= 0 ||
                ch == QLatin1Char('_') ||
                ch == QLatin1Char(':')) {
            result += ch;
        }
        else if (!result.isEmpty()) {
            if (result != QLatin1String("const"))
                return result;
            result.clear();
        }
    }
    return result;
}

/*!
  Returns a string representing the access specifier.
 */
QString RelatedClass::accessString() const
{
    switch (access_) {
    case Node::Protected:
        return QLatin1String("protected");
    case Node::Private:
        return QLatin1String("private");
    case Node::Public:
    default:
        break;
    }
    return QLatin1String("public");
}

/*!
  Returns the inheritance status.
 */
Node::Status Node::inheritedStatus() const
{
    Status parentStatus = Active;
    if (parent_)
        parentStatus = parent_->inheritedStatus();
    return qMin(status_, parentStatus);
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
    if (parent_ && safeness_ == parent_->inheritedThreadSafeness())
        return UnspecifiedSafeness;
    return safeness_;
}

/*!
  If this node has a parent, the parent's thread safeness
  value is returned. Otherwise, this node's thread safeness
  value is returned. Why?
 */
Node::ThreadSafeness Node::inheritedThreadSafeness() const
{
    if (parent_ && safeness_ == UnspecifiedSafeness)
        return parent_->inheritedThreadSafeness();
    return safeness_;
}


/*!
  If this node is a QML or JS type node, return a pointer to
  it. If it is a child of a QML or JS type node, return the
  pointer to its parent QMLor JS type node. Otherwise return
  0;
 */
QmlTypeNode* Node::qmlTypeNode()
{
    if (isQmlNode() || isJsNode()) {
        Node* n = this;
        while (n && !(n->isQmlType() || n->isJsType()))
            n = n->parent();
        if (n && (n->isQmlType() || n->isJsType()))
            return static_cast<QmlTypeNode*>(n);
    }
    return nullptr;
}

/*!
  If this node is a QML node, find its QML class node,
  and return a pointer to the C++ class node from the
  QML class node. That pointer will be null if the QML
  class node is a component. It will be non-null if
  the QML class node is a QML element.
 */
ClassNode* Node::declarativeCppNode()
{
    QmlTypeNode* qcn = qmlTypeNode();
    if (qcn)
        return qcn->classNode();
    return nullptr;
}

/*!
  Returns \c true if the node's status is Internal, or if its
  parent is a class with internal status.
 */
bool Node::isInternal() const
{
    if (status() == Internal)
        return true;
    if (parent() && parent()->status() == Internal)
        return true;
    return false;
}

/*!
  Returns a pointer to the root of the Tree this node is in.
 */
Aggregate *Node::root() const
{
    if (parent() == nullptr)
        return (this->isAggregate() ? static_cast<Aggregate*>(const_cast<Node*>(this)) : nullptr);
    Aggregate *t = parent();
    while (t->parent() != nullptr)
        t = t->parent();
    return t;
}

/*!
  Returns a pointer to the Tree this node is in.
 */
Tree* Node::tree() const
{
    return root()->tree();
}

/*!
  Sets the node's declaration location, its definition
  location, or both, depending on the suffix of the file
  name from the file path in location \a t.
 */
void Node::setLocation(const Location& t)
{
    QString suffix = t.fileSuffix();
    if (suffix == "h")
        declLocation_ = t;
    else if (suffix == "cpp")
        defLocation_ = t;
    else {
        declLocation_ = t;
        defLocation_ = t;
    }
}

/*!
  Adds this node to the shared comment node \a t.
 */
void Node::setSharedCommentNode(SharedCommentNode* t)
{
    sharedCommentNode_ = t;
    t->append(this);
}

/*!
  Returns true if this node is sharing a comment and the
  shared comment is not empty.
 */
bool Node::hasSharedDoc() const
{
    return (sharedCommentNode_ && sharedCommentNode_->hasDoc());
}

/*!
  Returns the CPP node's qualified name by prepending the
  namespaces name + "::" if there isw a namespace.
 */
QString Node::qualifyCppName()
{
    if (parent_ && parent_->isNamespace() && !parent_->name().isEmpty())
        return parent_->name() + "::" + name_;
    return name_;
}

/*!
  Return the name of this node qualified with the parent name
  and "::" if there is a parent name.
 */
QString Node::qualifyWithParentName()
{
    if (parent_ && !parent_->name().isEmpty())
        return parent_->name() + "::" + name_;
    return name_;
}


/*!
  Returns the QML node's qualified name by stripping off the
  "QML:" if present and prepending the logical module name.
 */
QString Node::qualifyQmlName()
{
    QString qualifiedName = name_;
    if (name_.startsWith(QLatin1String("QML:")))
        qualifiedName = name_.mid(4);
    qualifiedName = logicalModuleName() + "::" + name_;
    return qualifiedName;
}

/*!
  Returns the QML node's name after stripping off the
  "QML:" if present.
 */
QString Node::unqualifyQmlName()
{
    QString qmlTypeName = name_.toLower();
    if (qmlTypeName.startsWith(QLatin1String("qml:")))
        qmlTypeName = qmlTypeName.mid(4);
    return qmlTypeName;
}

/*!
  \class Aggregate
 */

/*!
  Calls delete for each child of this Aggregate that has this
  Aggregate as its parent. A child node that has some other
  Aggregate as its parent is deleted by that Aggregate's
  destructor.

  The destructor no longer deletes the collection of children
  by calling qDeleteAll() because the child list can contain
  pointers to children that have some other Aggregate as their
  parent. This is because of how the \e{\\relates} command is
  processed. An Aggregate can have a pointer to, for example,
  a FunctionNode in its child list, but that FunctionNode has
  a differen Aggregate as its parent because a \e{\\relates}
  command was used to relate it to that parent. In that case,
  the other Aggregate's destructor must delete that node.

  \note This function is the \b only place where delete is
  called to delete any subclass of Node.

  \note This strategy depends on the node tree being destroyed
  by calling delete on the root node of the tree. This happens
  in the destructor of class Tree.
 */
Aggregate::~Aggregate()
{
    enumChildren_.clear();
    nonfunctionMap_.clear();
    functionMap_.clear();
    for (int i = 0; i < children_.size(); ++i) {
        if ((children_[i] != nullptr) && (children_[i]->parent() == this))
            delete children_[i];
        children_[i] = nullptr;
    }
    children_.clear();
}

/*!
  If \a genus is \c{Node::DontCare}, find the first node in
  this node's child list that has the given \a name. If this
  node is a QML type, be sure to also look in the children
  of its property group nodes. Return the matching node or 0.

  If \a genus is either \c{Node::CPP} or \c {Node::QML}, then
  find all this node's children that have the given \a name,
  and return the one that satisfies the \a genus requirement.
 */
Node *Aggregate::findChildNode(const QString& name, Node::Genus genus, int findFlags) const
{
    if (genus == Node::DontCare) {
        Node *node = nonfunctionMap_.value(name);
        if (node)
            return node;
    } else {
        NodeList nodes = nonfunctionMap_.values(name);
        if (!nodes.isEmpty()) {
            for (int i = 0; i < nodes.size(); ++i) {
                Node* node = nodes.at(i);
                if (genus == node->genus()) {
                    if (findFlags & TypesOnly) {
                        if (!node->isTypedef()
                                && !node->isClassNode()
                                && !node->isQmlType()
                                && !node->isQmlBasicType()
                                && !node->isJsType()
                                && !node->isJsBasicType()
                                && !node->isEnumType())
                            continue;
                    } else if (findFlags & IgnoreModules && node->isModule())
                        continue;
                    return node;
                }
            }
        }
    }
    if (genus != Node::DontCare && this->genus() != genus)
        return nullptr;
    return functionMap_.value(name);
}

/*!
  Find all the child nodes of this node that are named
  \a name and return them in \a nodes.
 */
void Aggregate::findChildren(const QString &name, NodeVector &nodes) const
{
    nodes.clear();
    int nonfunctionCount = nonfunctionMap_.count(name);
    FunctionMap::const_iterator i = functionMap_.find(name);
    if (i != functionMap_.end()) {
        int functionCount = 0;
        FunctionNode *fn = i.value();
        while (fn != nullptr) {
            ++functionCount;
            fn = fn->nextOverload();
        }
        nodes.reserve(nonfunctionCount + functionCount);
        fn = i.value();
        while (fn != nullptr) {
            nodes.append(fn);
            fn = fn->nextOverload();
        }
    } else {
        nodes.reserve(nonfunctionCount);
    }
    if (nonfunctionCount > 0) {
        NodeMap::const_iterator i = nonfunctionMap_.find(name);
        while (i != nonfunctionMap_.end() && i.key() == name) {
            nodes.append(i.value());
            ++i;
        }
    }
}

/*!
  This function searches for a child node of this Aggregate,
  such that the child node has the spacified \a name and the
  function \a isMatch returns true for the node. The function
  passed must be one of the isXxx() functions in class Node
  that tests the node type.
 */
Node* Aggregate::findNonfunctionChild(const QString& name, bool (Node::*isMatch) () const)
{
    NodeList nodes = nonfunctionMap_.values(name);
    for (int i=0; i<nodes.size(); ++i) {
        Node* node = nodes.at(i);
        if ((node->*(isMatch))())
            return node;
    }
    return nullptr;
}

/*!
  Find a function node that is a child of this node, such that
  the function node has the specified \a name and \a parameters.
  If \a parameters is empty but no matching function is found
  that has no parameters, return the first non-internal primary
  function or overload, whether it has parameters or not.
 */
FunctionNode *Aggregate::findFunctionChild(const QString &name, const Parameters &parameters)
{
    FunctionMap::iterator i = functionMap_.find(name);
    if (i == functionMap_.end())
        return nullptr;
    FunctionNode *fn = i.value();

    if (parameters.isEmpty() && fn->parameters().isEmpty() && !fn->isInternal())
        return fn;

    while (fn != nullptr) {
        if (parameters.count() == fn->parameters().count() && !fn->isInternal()) {
            if (parameters.isEmpty())
                return fn;
            bool matched = true;
            for (int i = 0; i < parameters.count(); i++) {
                if (parameters.at(i).type() != fn->parameters().at(i).type()) {
                    matched = false;
                    break;
                }
            }
            if (matched)
                return fn;
        }
        fn = fn->nextOverload();
    }

    if (parameters.isEmpty()) {
        for (fn = i.value(); fn != nullptr; fn = fn->nextOverload())
            if (!fn->isInternal())
                return fn;
        return i.value();
    }
    return nullptr;
}

/*!
  Find the function node that is a child of this node, such
  that the function described has the same name and signature
  as the function described by the function node \a clone.
 */
FunctionNode *Aggregate::findFunctionChild(const FunctionNode *clone)
{
    FunctionNode *fn = functionMap_.value(clone->name());
    while (fn != nullptr) {
        if (isSameSignature(clone, fn))
            return fn;
        fn = fn->nextOverload();
    }
    return nullptr;
}

/*!
  Returns the list of keys from the primary function map.
 */
QStringList Aggregate::primaryKeys()
{
    return functionMap_.keys();
}

/*!
  Mark all child nodes that have no documentation as having
  private access and internal status. qdoc will then ignore
  them for documentation purposes.
 */
void Aggregate::markUndocumentedChildrenInternal()
{
    foreach (Node *child, children_) {
        if (!child->isSharingComment() && !child->hasDoc()) {
            if (!child->docMustBeGenerated()) {
                if (child->isFunction()) {
                    if (static_cast<FunctionNode*>(child)->hasAssociatedProperties())
                        continue;
                } else if (child->isTypedef()) {
                    if (static_cast<TypedefNode*>(child)->hasAssociatedEnum())
                        continue;
                }
                child->setAccess(Node::Private);
                child->setStatus(Node::Internal);
            }
        }
        if (child->isAggregate()) {
            static_cast<Aggregate*>(child)->markUndocumentedChildrenInternal();
        }
    }
}

/*!
  This is where we set the overload numbers for function nodes.
  \note Overload numbers for related non-members are handled
  separately.
 */
void Aggregate::normalizeOverloads()
{
    /*
      Ensure that none of the primary functions is inactive, private,
      or marked \e {overload}.
    */
    FunctionMap::Iterator i = functionMap_.begin();
    while (i != functionMap_.end()) {
        FunctionNode *fn = i.value();
        if (fn->isOverload()) {
            FunctionNode *primary = fn->findPrimaryFunction();
            if (primary) {
                primary->setNextOverload(fn);
                i.value() = primary;
                fn = primary;
            } else {
                fn->clearOverloadFlag();
            }
        }
        int count = 0;
        fn->setOverloadNumber(0);
        FunctionNode *internalFn = nullptr;
        while (fn != nullptr) {
            FunctionNode *next = fn->nextOverload();
            if (next) {
                if (next->isInternal()) {
                    // internal overloads are moved to a separate list
                    // and processed last
                    fn->setNextOverload(next->nextOverload());
                    next->setNextOverload(internalFn);
                    internalFn = next;
                } else {
                    next->setOverloadNumber(++count);
                }
                fn = fn->nextOverload();
            } else {
                fn->setNextOverload(internalFn);
                break;
            }
        }
        while (internalFn) {
            internalFn->setOverloadNumber(++count);
            internalFn = internalFn->nextOverload();
        }
        ++i; // process next function in function map.
    }
    /*
      Recursive part.
     */
    foreach (Node *n, children_) {
        if (n->isAggregate())
            static_cast<Aggregate*>(n)->normalizeOverloads();
    }
}

/*!
  Returns a const reference to the list of child nodes of this
  aggregate that are not function nodes. Duplicate nodes are
  removed from the list.
 */
const NodeList &Aggregate::nonfunctionList()
{
    std::list<Node*> list = nonfunctionMap_.values().toStdList();
    list.sort();
    list.unique();
    nonfunctionList_ = NodeList::fromStdList(list);
    return nonfunctionList_;
}

/*! \fn bool Aggregate::isAggregate() const
  Returns \c true because this is an inner node.
 */

/*!
  Returns \c true if the node is a class node or a QML type node
  that is marked as being a wrapper class or QML type, or if
  it is a member of a wrapper class or type.
 */
bool Node::isWrapper() const
{
    return (parent_ ? parent_->isWrapper() : false);
}

/*!
  Finds the enum type node that has \a enumValue as one of
  its enum values and returns a pointer to it. Returns 0 if
  no enum type node is found that has \a enumValue as one
  of its values.
 */
const EnumNode *Aggregate::findEnumNodeForValue(const QString &enumValue) const
{
    foreach (const Node *node, enumChildren_) {
        const EnumNode *en = static_cast<const EnumNode *>(node);
        if (en->hasItem(enumValue))
            return en;
    }
    return nullptr;
}

/*!
  Appends \a includeFile file to the list of include files.
 */
void Aggregate::addIncludeFile(const QString &includeFile)
{
    includeFiles_.append(includeFile);
}

/*!
  Sets the list of include files to \a includeFiles.
 */
void Aggregate::setIncludeFiles(const QStringList &includeFiles)
{
    includeFiles_ = includeFiles;
}

/*!
  f1 is always the clone
 */
bool Aggregate::isSameSignature(const FunctionNode *f1, const FunctionNode *f2)
{
    if (f1->parameters().count() != f2->parameters().count())
        return false;
    if (f1->isConst() != f2->isConst())
        return false;
    if (f1->isRef() != f2->isRef())
        return false;
    if (f1->isRefRef() != f2->isRefRef())
        return false;

    const Parameters &p1 = f1->parameters();
    const Parameters &p2 = f2->parameters();
    for (int i = 0; i < p1.count(); i++) {
        if (p1.at(i).hasType() && p2.at(i).hasType()) {
            QString t1 = p1.at(i).type();
            QString t2 = p2.at(i).type();

            if (t1.length() < t2.length())
                qSwap(t1, t2);

            /*
              ### hack for C++ to handle superfluous
              "Foo::" prefixes gracefully
             */
            if (t1 != t2 && t1 != (f2->parent()->name() + "::" + t2)) {
                // Accept a difference in the template parametters of the type if one
                // is omited (eg. "QAtomicInteger" == "QAtomicInteger<T>")
                auto ltLoc = t1.indexOf('<');
                auto gtLoc = t1.indexOf('>', ltLoc);
                if (ltLoc < 0 || gtLoc < ltLoc)
                    return false;
                t1.remove(ltLoc, gtLoc - ltLoc + 1);
                if (t1 != t2)
                    return false;
            }
        }
    }
    return true;
}

/*!
  This function is only called by addChild(), when the child is a
  FunctionNode. If the function map does not contain a function with
  the name in \a fn, \a fn is inserted into the function map. If the
  map already contains a function by that name, \a fn is appended to
  that function's linked list of overloads.

  \note A function's overloads appear in the linked list in the same
  order they were created. The first overload in the list is the first
  overload created. This order is maintained in the numbering of
  overloads. In other words, the first overload in the linked list has
  overload number 1, and the last overload in the list has overload
  number n, where n is the number of overloads not including the
  function in the function map.

  \not Adding a function increments the aggregate's function count,
  which is the total number of function nodes in the function map,
  including the overloads. The overloads are not inserted into the map
  but are in a linked list using the FunctionNode's nextOverload_
  pointer.

  \note The function's overload number and overload flag are set in
  normalizeOverloads().

  \sa normalizeOverloads()
 */
void Aggregate::addFunction(FunctionNode *fn)
{
    FunctionMap::iterator i = functionMap_.find(fn->name());
    if (i == functionMap_.end())
        functionMap_.insert(fn->name(), fn);
    else
        i.value()->appendOverload(fn);
    functionCount_++;
}

/*!
  When an Aggregate adopts a function that is a child of
  another Aggregate, the function is inserted into this
  Aggregate's function map, if the function's name is not
  already in the function map. If the function's name is
  already in the function map, do nothing. The overload
  link is already set correctly.
 */
void Aggregate::adoptFunction(FunctionNode *fn)
{
    FunctionMap::iterator i = functionMap_.find(fn->name());
    if (i == functionMap_.end())
        functionMap_.insert(fn->name(), fn);
    functionCount_++;
}

/*!
  Adds the \a child to this node's child map using \a title
  as the key. The \a child is not added to the child list
  again, because it is presumed to already be there. We just
  want to be able to find the child by its \a title.
 */
void Aggregate::addChildByTitle(Node* child, const QString& title)
{
    nonfunctionMap_.insertMulti(title, child);
}

/*!
  Adds the \a child to this node's child list and sets the child's
  parent pointer to this Aggregate. It then mounts the child with
  mountChild().

  The \a child is then added to this Aggregate's searchable maps
  and lists.

  \note This function does not test the child's parent pointer
  for null before changing it. If the child's parent pointer
  is not null, then it is being reparented. The child becomes
  a child of this Aggregate, but it also remains a child of
  the Aggregate that is it's old parent. But the child will
  only have one parent, and it will be this Aggregate. The is
  because of the \c relates command.

  \sa mountChild(), dismountChild()
 */
void Aggregate::addChild(Node *child)
{
    children_.append(child);
    child->setParent(this);
    child->setOutputSubdirectory(this->outputSubdirectory());
    child->setUrl(QString());
    child->setIndexNodeFlag(isIndexNode());
    if (child->isFunction()) {
        addFunction(static_cast<FunctionNode*>(child));
    }
    else {
        nonfunctionMap_.insertMulti(child->name(), child);
        if (child->isEnumType())
            enumChildren_.append(child);
    }
}

/*!
  This Aggregate becomes the adoptive parent of \a child. The
  \a child knows this Aggregate as its parent, but its former
  parent continues to have pointers to the child in its child
  list and in its searchable data structures. But the child is
  also added to the child list and searchable data structures
  of this Aggregate.

  The one caveat is that if the child being adopted is a function
  node, it's next overload pointer is not altered.
 */
void Aggregate::adoptChild(Node *child)
{
    if (child->parent() != this) {
        children_.append(child);
        child->setParent(this);
        if (child->isFunction()) {
            adoptFunction(static_cast<FunctionNode*>(child));
        }
        else {
            nonfunctionMap_.insertMulti(child->name(), child);
            if (child->isEnumType()) {
                enumChildren_.append(child);
            } else if (child->isSharedCommentNode()) {
                SharedCommentNode *scn = static_cast<SharedCommentNode*>(child);
                for (Node *n : scn->collective())
                    adoptChild(n);
            }
        }
    }
}

/*!
  Recursively sets the output subdirectory for children
 */
void Aggregate::setOutputSubdirectory(const QString &t)
{
    Node::setOutputSubdirectory(t);
    foreach (Node *n, children_)
        n->setOutputSubdirectory(t);
}

/*!
  Find the module (Qt Core, Qt GUI, etc.) to which the class belongs.
  We do this by obtaining the full path to the header file's location
  and examine everything between "src/" and the filename.  This is
  semi-dirty because we are assuming a particular directory structure.

  This function is only really useful if the class's module has not
  been defined in the header file with a QT_MODULE macro or with an
  \inmodule command in the documentation.
 */
QString Node::physicalModuleName() const
{
    if (!physicalModuleName_.isEmpty())
        return physicalModuleName_;

    QString path = location().filePath();
    QString pattern = QString("src") + QDir::separator();
    int start = path.lastIndexOf(pattern);

    if (start == -1)
        return QString();

    QString moduleDir = path.mid(start + pattern.size());
    int finish = moduleDir.indexOf(QDir::separator());

    if (finish == -1)
        return QString();

    QString physicalModuleName = moduleDir.left(finish);

    if (physicalModuleName == QLatin1String("corelib"))
        return QLatin1String("QtCore");
    else if (physicalModuleName == QLatin1String("uitools"))
        return QLatin1String("QtUiTools");
    else if (physicalModuleName == QLatin1String("gui"))
        return QLatin1String("QtGui");
    else if (physicalModuleName == QLatin1String("network"))
        return QLatin1String("QtNetwork");
    else if (physicalModuleName == QLatin1String("opengl"))
        return QLatin1String("QtOpenGL");
    else if (physicalModuleName == QLatin1String("svg"))
        return QLatin1String("QtSvg");
    else if (physicalModuleName == QLatin1String("sql"))
        return QLatin1String("QtSql");
    else if (physicalModuleName == QLatin1String("qtestlib"))
        return QLatin1String("QtTest");
    else if (moduleDir.contains("webkit"))
        return QLatin1String("QtWebKit");
    else if (physicalModuleName == QLatin1String("xml"))
        return QLatin1String("QtXml");
    else
        return QString();
}

/*!
  If this node has a child that is a QML property or JS property
  named \a n, return a pointer to that child. Otherwise, return \nullptr.
 */
QmlPropertyNode* Aggregate::hasQmlProperty(const QString& n) const
{
    NodeType goal = Node::QmlProperty;
    if (isJsNode())
        goal = Node::JsProperty;
    foreach (Node *child, children_) {
        if (child->nodeType() == goal) {
            if (child->name() == n)
                return static_cast<QmlPropertyNode*>(child);
        }
    }
    return nullptr;
}

/*!
  If this node has a child that is a QML property or JS property
  named \a n and that also matches \a attached, return a pointer
  to that child.
 */
QmlPropertyNode* Aggregate::hasQmlProperty(const QString& n, bool attached) const
{
    NodeType goal = Node::QmlProperty;
    if (isJsNode())
        goal = Node::JsProperty;
    foreach (Node *child, children_) {
        if (child->nodeType() == goal) {
            if (child->name() == n && child->isAttached() == attached)
                return static_cast<QmlPropertyNode*>(child);
        }
    }
    return nullptr;
}

/*!
  The FunctionNode \a fn is assumed to be a member function
  of this Aggregate. The function's name is looked up in the
  Aggregate's function map. It should be found because it is
  assumed that \a fn is in this Aggregate's function map. But
  in case it is not found, \c false is returned.

  Normally, the name will be found in the function map, and
  the value of the iterator is used to get the value, which
  is a pointer to another FunctionNode, which is not itself
  an overload. If that function has a non-null overload
  pointer, true is returned. Otherwise false is returned.

  This is a convenience function that you should not need to
  use.
 */
bool Aggregate::hasOverloads(const FunctionNode *fn) const
{
    FunctionMap::const_iterator i = functionMap_.find(fn->name());
    return (i == functionMap_.end() ? false : (i.value()->nextOverload() != nullptr));
}

/*!
  \class NamespaceNode
  \brief This class represents a C++ namespace.

  A namespace can be used in multiple C++ modules, so there
  can be a NamespaceNode for namespace Xxx in more than one
  Node tree.
 */

/*!
  If this is the global namespace node, remove all orphans
  from the child list before deleting anything.
 */
NamespaceNode::~NamespaceNode()
{
    for (int i = 0; i < children_.size(); ++i) {
        if (children_[i]->parent() != this)
            children_[i] = nullptr;
    }
}

/*!
  Returns true if this namespace is to be documented in the
  current module. There can be elements declared in this
  namespace spread over multiple modules. Those elements are
  documented in the modules where they are declared, but they
  are linked to from the namespace page in the module where
  the namespace itself is documented.
 */
bool NamespaceNode::isDocumentedHere() const
{
    return whereDocumented_ == tree()->camelCaseModuleName();
}

/*!
  Returns true if this namespace node contains at least one
  child that has documentation and is not private or internal.
 */
bool NamespaceNode::hasDocumentedChildren() const
{
    foreach (Node *n, children_) {
        if (n->isInAPI())
            return true;
    }
    return false;
}

/*!
  Report qdoc warning for each documented child in a namespace
  that is not documented. This function should only be called
  when the namespace is not documented.
 */
void NamespaceNode::reportDocumentedChildrenInUndocumentedNamespace() const
{
    foreach (Node *n, children_) {
        if (n->isInAPI()) {
            QString msg1 = n->name();
            if (n->isFunction())
                msg1 += "()";
            msg1 += tr(" is documented, but namespace %1 is not documented in any module.").arg(name());
            QString msg2 = tr("Add /*! '\\%1 %2' ... */ or remove the qdoc comment marker (!) at that line number.").arg(COMMAND_NAMESPACE).arg(name());

            n->doc().location().warning(msg1, msg2);
        }
    }
}

/*!
  Returns true if this namespace node is not private and
  contains at least one public child node with documentation.
 */
bool NamespaceNode::docMustBeGenerated() const
{
    if (isInAPI())
        return true;
    return (hasDocumentedChildren() ? true : false);
}

/*!
  Returns a const reference to the namespace node's list of
  included children, which contains pointers to all the child
  nodes of other namespace nodes that have the same name as
  this namespace node. The list is built after the prepare
  phase has been run but just before the generate phase. It
  is buils by QDocDatabase::resolveNamespaces().

  \sa QDocDatabase::resolveNamespaces()
 */
const NodeList &NamespaceNode::includedChildren() const
{
    return includedChildren_;
}

/*!
  This function is only called from QDocDatabase::resolveNamesapces().

  \sa includedChildren(), QDocDatabase::resolveNamespaces()
 */
void NamespaceNode::includeChild(Node *child)
{
    includedChildren_.append(child);
}

/*!
  \class ClassNode
  \brief This class represents a C++ class.
 */

/*!
  Adds the base class \a node to this class's list of base
  classes. The base class has the specified \a access. This
  is a resolved base class.
 */
void ClassNode::addResolvedBaseClass(Access access, ClassNode* node)
{
    bases_.append(RelatedClass(access, node));
    node->derived_.append(RelatedClass(access, this));
}

/*!
  Adds the derived class \a node to this class's list of derived
  classes. The derived class inherits this class with \a access.
 */
void ClassNode::addDerivedClass(Access access, ClassNode* node)
{
    derived_.append(RelatedClass(access, node));
}

/*!
  Add an unresolved base class to this class node's list of
  base classes. The unresolved base class will be resolved
  before the generate phase of qdoc. In an unresolved base
  class, the pointer to the base class node is 0.
 */
void ClassNode::addUnresolvedBaseClass(Access access,
                                       const QStringList& path,
                                       const QString& signature)
{
    bases_.append(RelatedClass(access, path, signature));
}

/*!
  Add an unresolved \c using clause to this class node's list
  of \c using clauses. The unresolved \c using clause will be
  resolved before the generate phase of qdoc. In an unresolved
  \c using clause, the pointer to the function node is 0.
 */
void ClassNode::addUnresolvedUsingClause(const QString& signature)
{
    usingClauses_.append(UsingClause(signature));
}

/*!
  A base class of this class node was private or internal.
  That node's list of \a bases is traversed in this function.
  Each of its public base classes is promoted to be a base
  class of this node for documentation purposes. For each
  private or internal class node in \a bases, this function
  is called recursively with the list of base classes from
  that private or internal class node.
 */
void ClassNode::promotePublicBases(const QList<RelatedClass>& bases)
{
    if (!bases.isEmpty()) {
        for (int i = bases.size() - 1; i >= 0; --i) {
            ClassNode* bc = bases.at(i).node_;
            if (bc == nullptr)
                bc = QDocDatabase::qdocDB()->findClassNode(bases.at(i).path_);
            if (bc != nullptr) {
                if (bc->isPrivate() || bc->isInternal())
                    promotePublicBases(bc->baseClasses());
                else
                    bases_.append(bases.at(i));
            }
        }
    }
}

/*!
  Remove private and internal bases classes from this class's list
  of base classes. When a base class is removed from the list, add
  its base classes to this class's list of base classes.
 */
void ClassNode::removePrivateAndInternalBases()
{
    int i;
    i = 0;
    QSet<ClassNode *> found;

    // Remove private and duplicate base classes.
    while (i < bases_.size()) {
        ClassNode* bc = bases_.at(i).node_;
        if (bc == nullptr)
            bc = QDocDatabase::qdocDB()->findClassNode(bases_.at(i).path_);
        if (bc != nullptr && (bc->isPrivate() || bc->isInternal() || found.contains(bc))) {
            RelatedClass rc = bases_.at(i);
            bases_.removeAt(i);
            ignoredBases_.append(rc);
            promotePublicBases(bc->baseClasses());
        }
        else {
            ++i;
        }
        found.insert(bc);
    }

    i = 0;
    while (i < derived_.size()) {
        ClassNode* dc = derived_.at(i).node_;
        if (dc != nullptr && (dc->isPrivate() || dc->isInternal())) {
            derived_.removeAt(i);
            const QList<RelatedClass> &dd = dc->derivedClasses();
            for (int j = dd.size() - 1; j >= 0; --j)
                derived_.insert(i, dd.at(j));
        }
        else {
            ++i;
        }
    }
}

/*!
 */
void ClassNode::resolvePropertyOverriddenFromPtrs(PropertyNode* pn)
{
    QList<RelatedClass>::const_iterator bc = baseClasses().constBegin();
    while (bc != baseClasses().constEnd()) {
        ClassNode* cn = bc->node_;
        if (cn) {
            Node* n = cn->findNonfunctionChild(pn->name(), &Node::isProperty);
            if (n) {
                PropertyNode* baseProperty = static_cast<PropertyNode*>(n);
                cn->resolvePropertyOverriddenFromPtrs(baseProperty);
                pn->setOverriddenFrom(baseProperty);
            }
            else
                cn->resolvePropertyOverriddenFromPtrs(pn);
        }
        ++bc;
    }
}

/*!
  Search the child list to find the property node with the
  specified \a name.
 */
PropertyNode* ClassNode::findPropertyNode(const QString& name)
{
    Node* n = findNonfunctionChild(name, &Node::isProperty);

    if (n)
        return static_cast<PropertyNode*>(n);

    PropertyNode* pn = nullptr;

    const QList<RelatedClass> &bases = baseClasses();
    if (!bases.isEmpty()) {
        for (int i = 0; i < bases.size(); ++i) {
            ClassNode* cn = bases[i].node_;
            if (cn) {
                pn = cn->findPropertyNode(name);
                if (pn)
                    break;
            }
        }
    }
    const QList<RelatedClass>& ignoredBases = ignoredBaseClasses();
    if (!ignoredBases.isEmpty()) {
        for (int i = 0; i < ignoredBases.size(); ++i) {
            ClassNode* cn = ignoredBases[i].node_;
            if (cn) {
                pn = cn->findPropertyNode(name);
                if (pn)
                    break;
            }
        }
    }

    return pn;
}

/*!
  This function does a recursive search of this class node's
  base classes looking for one that has a QML element. If it
  finds one, it returns the pointer to that QML element. If
  it doesn't find one, it returns null.
 */
QmlTypeNode* ClassNode::findQmlBaseNode()
{
    QmlTypeNode* result = nullptr;
    const QList<RelatedClass>& bases = baseClasses();

    if (!bases.isEmpty()) {
        for (int i = 0; i < bases.size(); ++i) {
            ClassNode* cn = bases[i].node_;
            if (cn && cn->qmlElement()) {
                return cn->qmlElement();
            }
        }
        for (int i = 0; i < bases.size(); ++i) {
            ClassNode* cn = bases[i].node_;
            if (cn) {
                result = cn->findQmlBaseNode();
                if (result != nullptr) {
                    return result;
                }
            }
        }
    }
    return result;
}

/*!
  \a fn is an overriding function in this class or in a class
  derived from this class. Find the node for the function that
  \a fn overrides in this class's children or in one of this
  class's base classes.

  \returns a pointer to the overridden function, or \nullptr.

  This should be revised because clang provides the path to the
  overridden function. mws 15/12/2018
 */
FunctionNode* ClassNode::findOverriddenFunction(const FunctionNode* fn)
{
    QList<RelatedClass>::Iterator bc = bases_.begin();
    while (bc != bases_.end()) {
        ClassNode *cn = bc->node_;
        if (cn == nullptr) {
            cn = QDocDatabase::qdocDB()->findClassNode(bc->path_);
            bc->node_ = cn;
        }
        if (cn != nullptr) {
            FunctionNode *result = cn->findFunctionChild(fn);
            if (result != nullptr && !result->isInternal() && !result->isNonvirtual() && result->hasDoc())
                return result;
            result = cn->findOverriddenFunction(fn);
            if (result != nullptr && !result->isNonvirtual())
                return result;
        }
        ++bc;
    }
    return nullptr;
}

/*!
  \a fn is an overriding function in this class or in a class
  derived from this class. Find the node for the property that
  \a fn overrides in this class's children or in one of this
  class's base classes.

  \returns a pointer to the overridden function, or \nullptr.
 */
PropertyNode* ClassNode::findOverriddenProperty(const FunctionNode* fn)
{
    QList<RelatedClass>::Iterator bc = bases_.begin();
    while (bc != bases_.end()) {
        ClassNode *cn = bc->node_;
        if (cn == nullptr) {
            cn = QDocDatabase::qdocDB()->findClassNode(bc->path_);
            bc->node_ = cn;
        }
        if (cn != nullptr) {
            const NodeList &children = cn->childNodes();
            NodeList::const_iterator i = children.begin();
            while (i != children.end()) {
                if ((*i)->isProperty()) {
                    PropertyNode *pn = static_cast<PropertyNode*>(*i);
                    if (pn->name() == fn->name() || pn->hasAccessFunction(fn->name())) {
                        if (pn->hasDoc())
                            return pn;
                    }
                }
                i++;
            }
            PropertyNode *result = cn->findOverriddenProperty(fn);
            if (result != nullptr)
                return result;
        }
        ++bc;
    }
    return nullptr;
}

/*!
  Returns true if the class or struct represented by this class
  node must be documented. If this function returns true, then
  qdoc must find a qdoc comment for this class. If it returns
  false, then the class need not be documented.
 */
bool ClassNode::docMustBeGenerated() const
{
    if (!hasDoc() || isPrivate() || isInternal() || isDontDocument())
        return false;
    if (declLocation().fileName().endsWith(QLatin1String("_p.h")) && !hasDoc())
        return false;

    return true;
}

/*!
  \class Headerode
  \brief This class represents a C++ header file.
 */

HeaderNode::HeaderNode(Aggregate* parent, const QString& name) : Aggregate(HeaderFile, parent, name)
{
    // Add the include file with enclosing angle brackets removed
    if (name.startsWith(QChar('<')) && name.length() > 2)
        Aggregate::addIncludeFile(name.mid(1).chopped(1));
    else
        Aggregate::addIncludeFile(name);
}

/*!
  Returns true if this header file node is not private and
  contains at least one public child node with documentation.
 */
bool HeaderNode::docMustBeGenerated() const
{
    if (isInAPI())
        return true;
    return (hasDocumentedChildren() ? true : false);
}

/*!
  Returns true if this header file node contains at least one
  child that has documentation and is not private or internal.
 */
bool HeaderNode::hasDocumentedChildren() const
{
    foreach (Node *n, children_) {
        if (n->isInAPI())
            return true;
    }
    return false;
}

/*!
  \class PageNode
 */

/*! \fn QString PageNode::title() const
  Returns the node's title, which is used for the page title.
 */

/*! \fn QString PageNode::subtitle() const
  Returns the node's subtitle, which may be empty.
 */

/*!
  Returns the node's full title, which is usually whatever
  title() returns, but for some cases the full title migth
  be different from title(), so this might require changing,
  because currently it just returns the title().

  mws 13/07/2018. This function used to test the node subtype
  for File or Image and append text to the title(), but there
  are no node subtypes now, so it can't do that. There are no
  node type values for File and Image either. Files and images
  are used in examples, but the ExampleNode's example files
  and example images are stored as lists of path strings.
 */
QString PageNode::fullTitle() const
{
    return title();
}

/*!
  Sets the node's \a title, which is used for the page title.
  Returns true.
 */
bool PageNode::setTitle(const QString &title)
{
    title_ = title;
    parent()->addChildByTitle(this, title);
    return true;
}

/*!
  \fn bool PageNode::setSubtitle(const QString &subtitle)
  Sets the node's \a subtitle. Returns true;
 */

/*! \f void Node::markInternal()
  Sets the node's access to Private and its status to Internal.
 */

/*!
  \class EnumNode
 */

/*!
  Add \a item to the enum type's item list.
 */
void EnumNode::addItem(const EnumItem& item)
{
    items_.append(item);
    names_.insert(item.name());
}

/*!
  Returns the access level of the enumeration item named \a name.
  Apparently it is private if it has been omitted by qdoc's
  omitvalue command. Otherwise it is public.
 */
Node::Access EnumNode::itemAccess(const QString &name) const
{
    if (doc().omitEnumItemNames().contains(name))
        return Private;
    return Public;
}

/*!
  Returns the enum value associated with the enum \a name.
 */
QString EnumNode::itemValue(const QString &name) const
{
    foreach (const EnumItem &item, items_) {
        if (item.name() == name)
            return item.value();
    }
    return QString();
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *EnumNode::clone(Aggregate *parent)
{
    EnumNode *en = new EnumNode(*this); // shallow copy
    en->setParent(nullptr);
    parent->addChild(en);
    return en;
}

/*!
  \class TypedefNode
 */

/*!
 */
void TypedefNode::setAssociatedEnum(const EnumNode *enume)
{
    associatedEnum_ = enume;
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *TypedefNode::clone(Aggregate *parent)
{
    TypedefNode *tn = new TypedefNode(*this); // shallow copy
    tn->setParent(nullptr);
    parent->addChild(tn);
    return tn;
}

/*!
  \class TypeAliasNode
 */

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *TypeAliasNode::clone(Aggregate *parent)
{
    TypeAliasNode *tan = new TypeAliasNode(*this); // shallow copy
    tan->setParent(nullptr);
    parent->addChild(tan);
    return tan;
}

/*!
  \class FunctionNode

  This node is used to represent any kind of function being
  documented. It can represent a C++ class member function,
  a C++ global function, a QML method, a javascript method,
  or a macro, with or without parameters.

  A C++ function can be a signal a slot, a constructor of any
  kind, a destructor, a copy or move assignment operator, or
  just a plain old member function or global function.

  A QML or javascript method can be a plain old method, or a
  signal or signal handler.

  If the function is not an overload, its overload flag is
  false. If it is an overload, its overload flag is true.
  If it is not an overload but it has overloads, its next
  overload pointer will point to an overload function. If it
  is an overload function, its overload flag is true, and it
  may or may not have a non-null next overload pointer.

  So all the overloads of a function are in a linked list
  using the next overload pointer. If a function has no
  overloads, its overload flag is false and its overload
  pointer is null.

  The function node also has an overload number. If the
  node's overload flag is set, this overload number is
  positive; otherwise, the overload number is 0.
 */

/*!
  Construct a function node for a C++ function. It's parent
  is \a parent, and it's name is \a name.

  \note The function node's overload flag is set to false, and
  its overload number is set to 0. These data members are set
  in normalizeOverloads(), when all the overloads are known.
 */
FunctionNode::FunctionNode(Aggregate *parent, const QString& name)
    : Node(Function, parent, name),
      const_(false),
      static_(false),
      reimpFlag_(false),
      attached_(false),
      overloadFlag_(false),
      isFinal_(false),
      isOverride_(false),
      isRef_(false),
      isRefRef_(false),
      isInvokable_(false),
      metaness_(Plain),
      virtualness_(NonVirtual),
      overloadNumber_(0),
      nextOverload_(nullptr)
{
    // nothing
}

/*!
  Construct a function node for a QML method or signal, specified
  by ther Metaness value \a type. It's parent is \a parent, and
  it's name is \a name. If \a attached is true, it is an attached
  method or signal.

  \note The function node's overload flag is set to false, and
  its overload number is set to 0. These data members are set
  in normalizeOverloads(), when all the overloads are known.
 */
FunctionNode::FunctionNode(Metaness kind, Aggregate *parent, const QString& name, bool attached)
    : Node(Function, parent, name),
      const_(false),
      static_(false),
      reimpFlag_(false),
      attached_(attached),
      overloadFlag_(false),
      isFinal_(false),
      isOverride_(false),
      isRef_(false),
      isRefRef_(false),
      isInvokable_(false),
      metaness_(kind),
      virtualness_(NonVirtual),
      overloadNumber_(0),
      nextOverload_(nullptr)
{
    setGenus(getGenus(metaness_));
    if (!isCppNode() && name.startsWith("__"))
        setStatus(Internal);
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *FunctionNode::clone(Aggregate *parent)
{
    FunctionNode *fn = new FunctionNode(*this); // shallow copy
    fn->setParent(nullptr);
    fn->setNextOverload(nullptr);
    parent->addChild(fn);
    return fn;
}

/*!
  Returns this function's virtualness value as a string
  for use as an attribute value in index files.
 */
QString FunctionNode::virtualness() const
{
    switch (virtualness_) {
    case FunctionNode::NormalVirtual:
        return QLatin1String("virtual");
    case FunctionNode::PureVirtual:
        return QLatin1String("pure");
    case FunctionNode::NonVirtual:
    default:
        break;
    }
    return QLatin1String("non");
}

/*!
  Sets the function node's virtualness value based on the value
  of string \a t, which is the value of the function's \e{virtual}
  attribute in an index file. If \a t is \e{pure}, and if the
  parent() is a C++ class, set the parent's \e abstract flag to
  \c {true}.
 */
void FunctionNode::setVirtualness(const QString& t)
{
    if (t == QLatin1String("non"))
        virtualness_ = NonVirtual;
    else if (t == QLatin1String("virtual"))
        virtualness_ = NormalVirtual;
    else if (t == QLatin1String("pure")) {
        virtualness_ = PureVirtual;
        if (parent() && parent()->isClassNode())
            parent()->setAbstract(true);
    }
}

static QMap<QString, FunctionNode::Metaness> metanessMap_;
static void buildMetanessMap()
{
    metanessMap_["plain"] = FunctionNode::Plain;
    metanessMap_["signal"] = FunctionNode::Signal;
    metanessMap_["slot"] = FunctionNode::Slot;
    metanessMap_["constructor"] = FunctionNode::Ctor;
    metanessMap_["copy-constructor"] = FunctionNode::CCtor;
    metanessMap_["move-constructor"] = FunctionNode::MCtor;
    metanessMap_["destructor"] = FunctionNode::Dtor;
    metanessMap_["macro"] = FunctionNode::MacroWithParams;
    metanessMap_["macrowithparams"] = FunctionNode::MacroWithParams;
    metanessMap_["macrowithoutparams"] = FunctionNode::MacroWithoutParams;
    metanessMap_["copy-assign"] = FunctionNode::CAssign;
    metanessMap_["move-assign"] = FunctionNode::MAssign;
    metanessMap_["native"] = FunctionNode::Native;
    metanessMap_["qmlsignal"] = FunctionNode::QmlSignal;
    metanessMap_["qmlsignalhandler"] = FunctionNode::QmlSignalHandler;
    metanessMap_["qmlmethod"] = FunctionNode::QmlMethod;
    metanessMap_["jssignal"] = FunctionNode::JsSignal;
    metanessMap_["jssignalhandler"] = FunctionNode::JsSignalHandler;
    metanessMap_["jsmethos"] = FunctionNode::JsMethod;
}

static QMap<QString, FunctionNode::Metaness> topicMetanessMap_;
static void buildTopicMetanessMap()
{
    topicMetanessMap_["fn"] = FunctionNode::Plain;
    topicMetanessMap_["qmlsignal"] = FunctionNode::QmlSignal;
    topicMetanessMap_["qmlattachedsignal"] = FunctionNode::QmlSignal;
    topicMetanessMap_["qmlmethod"] = FunctionNode::QmlMethod;
    topicMetanessMap_["qmlattachedmethod"] = FunctionNode::QmlMethod;
    topicMetanessMap_["jssignal"] = FunctionNode::JsSignal;
    topicMetanessMap_["jsattachedsignal"] = FunctionNode::JsSignal;
    topicMetanessMap_["jsmethod"] = FunctionNode::JsMethod;
    topicMetanessMap_["jsattachedmethod"] = FunctionNode::JsMethod;
}

/*!
  Determines the Genus value for this FunctionNode given the
  Metaness value \a t. Returns the Genus value. \a t must be
  one of the values of Metaness. If not, Node::DontCare is
  returned.
 */
Node::Genus FunctionNode::getGenus(FunctionNode::Metaness t)
{
    switch (t) {
        case FunctionNode::Plain:
        case FunctionNode::Signal:
        case FunctionNode::Slot:
        case FunctionNode::Ctor:
        case FunctionNode::Dtor:
        case FunctionNode::CCtor:
        case FunctionNode::MCtor:
        case FunctionNode::MacroWithParams:
        case FunctionNode::MacroWithoutParams:
        case FunctionNode::Native:
        case FunctionNode::CAssign:
        case FunctionNode::MAssign:
            return Node::CPP;
        case FunctionNode::QmlSignal:
        case FunctionNode::QmlSignalHandler:
        case FunctionNode::QmlMethod:
            return Node::QML;
        case FunctionNode::JsSignal:
        case FunctionNode::JsSignalHandler:
        case FunctionNode::JsMethod:
            return Node::JS;
    }
    return Node::DontCare;
}

/*!
  This static function converts the string \a t to an enum
  value for the kind of function named by \a t.
 */
FunctionNode::Metaness FunctionNode::getMetaness(const QString& t)
{
    if (metanessMap_.isEmpty())
        buildMetanessMap();
    return metanessMap_[t];
}

/*!
  This static function converts the topic string \a t to an enum
  value for the kind of function this FunctionNode represents.
 */
FunctionNode::Metaness FunctionNode::getMetanessFromTopic(const QString& t)
{
    if (topicMetanessMap_.isEmpty())
        buildTopicMetanessMap();
    return topicMetanessMap_[t];
}

/*!
  Sets the function node's Metaness value based on the value
  of string \a t, which is the value of the function's "meta"
  attribute in an index file. Returns the Metaness value
 */
FunctionNode::Metaness FunctionNode::setMetaness(const QString& t)
{
    metaness_ = getMetaness(t);
    return metaness_;
}

/*!
  If this function node's metaness is \a from, change the
  metaness to \a to and return \c true. Otherwise return
  false. This function is used to change Qml function node
  metaness values to Javascript function node metaness,
  values because these nodes are created as Qml function
  nodes before it is discovered that what the function node
  represents is not a Qml function but a javascript function.

  Note that if the function returns true, which means the node
  type was indeed changed, then the node's Genus is also changed
  from QML to JS.

  The function also works in the other direction, but there is
  no use case for that.
 */
bool FunctionNode::changeMetaness(Metaness from, Metaness to)
{
    if (metaness_ == from) {
        metaness_ = to;
        switch (to) {
          case QmlSignal:
          case QmlSignalHandler:
          case QmlMethod:
              setGenus(Node::QML);
              break;
          case JsSignal:
          case JsSignalHandler:
          case JsMethod:
              setGenus(Node::JS);
              break;
          default:
              setGenus(Node::CPP);
              break;
        }
        return true;
    }
    return false;
}

/*! \fn void FunctionNode::setOverloadNumber(unsigned char n)
  Sets the function node's overload number to \a n. If \a n
  is 0, the function node's overload flag is set to false. If
  \a n is greater than 0, the overload flag is set to true.
 */
void FunctionNode::setOverloadNumber(signed short n)
{
    overloadNumber_ = n;
    overloadFlag_ = (n > 0) ? true : false;
}

/*!
  If this function's next overload pointer is null, set it to
  \a fn. Otherwise continue down the overload list by calling
  this function recursively for the next overload.

  Although this function appends an overload function to the list of
  overloads for this function's name, it does not set the function's
  overload number or it's overload flag. If the function has the
  \c{\\overload} in its qdoc comment, that will set the overload
  flag. But qdoc treats the \c{\\overload} command as a hint that the
  function should be documented as an overload. The hint is almost
  always correct, but qdoc reserves the right to decide which function
  should be the primary function and which functions are the overloads.
  These decisions are made in Aggregate::normalizeOverloads().
 */
void FunctionNode::appendOverload(FunctionNode *fn)
{
    if (nextOverload_ == nullptr)
        nextOverload_ = fn;
    else
        nextOverload_->appendOverload(fn);
}

/*!
  This function assumes that this FunctionNode is marked as an
  overload function. It asks if the next overload is marked as
  an overload. If not, then remove that FunctionNode from the
  overload list and return it. Otherwise call this function
  recursively for the next overload.
 */
FunctionNode *FunctionNode::findPrimaryFunction()
{
    if (nextOverload_ != nullptr) {
        if (!nextOverload_->isOverload()) {
            FunctionNode *t = nextOverload_;
            nextOverload_ = t->nextOverload();
            t->setNextOverload(nullptr);
            return t;
        }
        return nextOverload_->findPrimaryFunction();
    }
    return nullptr;
}

/*!
  \fn void FunctionNode::setReimpFlag()

  Sets the function node's reimp flag to \c true, which means
  the \e {\\reimp} command was used in the qdoc comment. It is
  supposed to mean that the function reimplements a virtual
  function in a base class.
 */

/*!
  Returns a string representing the kind of function this
  Function node represents, which depends on the Metaness
  value.
 */
QString FunctionNode::kindString() const
{
    switch (metaness_) {
        case FunctionNode::QmlSignal:
            return "QML signal";
        case FunctionNode::QmlSignalHandler:
            return "QML signal handler";
        case FunctionNode::QmlMethod:
            return "QML method";
        case FunctionNode::JsSignal:
            return "JS signal";
        case FunctionNode::JsSignalHandler:
            return "JS signal handler";
        case FunctionNode::JsMethod:
            return "JS method";
        default:
            return "function";
    }
}

/*!
  Returns a string representing the Metaness enum value for
  this function. It is used in index files.
 */
QString FunctionNode::metanessString() const
{
    switch (metaness_) {
    case FunctionNode::Plain:
        return "plain";
    case FunctionNode::Signal:
        return "signal";
    case FunctionNode::Slot:
        return "slot";
    case FunctionNode::Ctor:
        return "constructor";
    case FunctionNode::CCtor:
        return "copy-constructor";
    case FunctionNode::MCtor:
        return "move-constructor";
    case FunctionNode::Dtor:
        return "destructor";
    case FunctionNode::MacroWithParams:
        return "macrowithparams";
    case FunctionNode::MacroWithoutParams:
        return "macrowithoutparams";
    case FunctionNode::Native:
        return "native";
    case FunctionNode::CAssign:
        return "copy-assign";
    case FunctionNode::MAssign:
        return "move-assign";
    case FunctionNode::QmlSignal:
        return "qmlsignal";
    case FunctionNode::QmlSignalHandler:
        return "qmlsignalhandler";
    case FunctionNode::QmlMethod:
        return "qmlmethod";
    case FunctionNode::JsSignal:
        return "jssignal";
    case FunctionNode::JsSignalHandler:
        return "jssignalhandler";
    case FunctionNode::JsMethod:
        return "jsmethod";
    default:
        return "plain";
    }
    return QString();
}

/*!
  Adds the "associated" property \a p to this function node.
  The function might be the setter or getter for a property,
  for example.
 */
void FunctionNode::addAssociatedProperty(PropertyNode *p)
{
    associatedProperties_.append(p);
}

/*!
  Returns true if this function has at least one property
  that is active, i.e. at least one property that is not
  obsolete.
 */
bool FunctionNode::hasActiveAssociatedProperty() const
{
    if (associatedProperties_.isEmpty())
        return false;
    foreach (const Node *p, associatedProperties_) {
        if (!p->isObsolete())
            return true;
    }
    return false;
}

/*! \fn unsigned char FunctionNode::overloadNumber() const
  Returns the overload number for this function.
 */

/*!
  Reconstructs and returns the function's signature. If \a values
  is true, the default values of the parameters are included, if
  present.
 */
QString FunctionNode::signature(bool values, bool noReturnType) const
{
    QString result;
    if (!noReturnType && !returnType().isEmpty())
        result = returnType() + QLatin1Char(' ');
    result += name();
    if (!isMacroWithoutParams()) {
        result += QLatin1Char('(') + parameters_.signature(values) + QLatin1Char(')');
        if (isMacro())
            return result;
    }
    if (isConst())
        result += " const";
    if (isRef())
        result += " &";
    else if (isRefRef())
        result += " &&";
    return result;
}

/*!
  Returns true if function \a fn has role \a r for this
  property.
 */
PropertyNode::FunctionRole PropertyNode::role(const FunctionNode* fn) const
{
    for (int i=0; i<4; i++) {
        if (functions_[i].contains(const_cast<FunctionNode*>(fn)))
            return (FunctionRole) i;
    }
    return Notifier;
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *VariableNode::clone(Aggregate *parent)
{
    VariableNode *vn = new VariableNode(*this); // shallow copy
    vn->setParent(nullptr);
    parent->addChild(vn);
    return vn;
}

/*!
  Print some debugging stuff.
 */
void FunctionNode::debug() const
{
    qDebug("QML METHOD %s returnType_ %s parentPath_ %s",
           qPrintable(name()), qPrintable(returnType_), qPrintable(parentPath_.join(' ')));
}

/*!
  Compares this FunctionNode to the FunctionNode pointed to
  by \a fn. Returns true if they describe the same function.
 */
bool FunctionNode::compare(const FunctionNode *fn) const
{
    if (fn == nullptr)
        return false;
    if (metaness() != fn->metaness())
        return false;
    if (parent() != fn->parent())
        return false;
    if (returnType_ != fn->returnType())
        return false;
    if (isConst() != fn->isConst())
        return false;
    if (isAttached() != fn->isAttached())
        return false;
    const Parameters &p = fn->parameters();
    if (parameters_.count() != p.count())
        return false;
    if (!p.isEmpty()) {
        for (int i = 0; i < p.count(); ++i) {
            if (parameters_.at(i).type() != p.at(i).type())
                return false;
        }
    }
    return true;
}

/*!
  In some cases, it is ok for a public function to be not documented.
  For example, the macro Q_OBJECT adds several functions to the API of
  a class, but these functions are normally not meant to be documented.
  So if a function node doesn't have documentation, then if its name is
  in the list of functions that it is ok not to document, this function
  returns true. Otherwise, it returns false.

  These are the member function names added by macros.  Usually they
  are not documented, but they can be documented, so this test avoids
  reporting an error if they are not documented.

  But maybe we should generate a standard text for each of them?
 */
bool FunctionNode::isIgnored() const
{
    if (!hasDoc() && !hasSharedDoc()) {
        if (name().startsWith(QLatin1String("qt_")) ||
            name() == QLatin1String("metaObject") ||
            name() == QLatin1String("tr") ||
            name() == QLatin1String("trUtf8") ||
            name() == QLatin1String("d_func")) {
            return true;
        }
        QString s = signature(false, false);
        if (s.contains(QLatin1String("enum_type")) && s.contains(QLatin1String("operator|")))
            return true;
    }
    return false;
}

/*!
  Returns true if this function has overloads. Otherwise false.
  First, if this function node's overload pointer is not nullptr,
  return true. Next, if this function node's overload flag is true
  return true. Finally, if this function's parent Aggregate has a
  function by the same name as this one in its function map and
  that function has overloads, return true. Otherwise return false.

  There is a failsafe way to test it under any circumstances.
 */
bool FunctionNode::hasOverloads() const
{
    if (nextOverload_ != nullptr)
        return true;
    if (overloadFlag_)
        return true;
    if (parent())
        return parent()->hasOverloads(this);
    return false;
}

/*!
  \class PropertyNode

  This class describes one instance of using the Q_PROPERTY macro.
 */

/*!
  The constructor sets the \a parent and the \a name, but
  everything else is set to default values.
 */
PropertyNode::PropertyNode(Aggregate *parent, const QString& name)
    : Node(Property, parent, name),
      stored_(FlagValueDefault),
      designable_(FlagValueDefault),
      scriptable_(FlagValueDefault),
      writable_(FlagValueDefault),
      user_(FlagValueDefault),
      const_(false),
      final_(false),
      revision_(-1),
      overrides_(nullptr)
{
    // nothing
}

/*!
  Sets this property's \e {overridden from} property to
  \a baseProperty, which indicates that this property
  overrides \a baseProperty. To begin with, all the values
  in this property are set to the corresponding values in
  \a baseProperty.

  We probably should ensure that the constant and final
  attributes are not being overridden improperly.
 */
void PropertyNode::setOverriddenFrom(const PropertyNode* baseProperty)
{
    for (int i = 0; i < NumFunctionRoles; ++i) {
        if (functions_[i].isEmpty())
            functions_[i] = baseProperty->functions_[i];
    }
    if (stored_ == FlagValueDefault)
        stored_ = baseProperty->stored_;
    if (designable_ == FlagValueDefault)
        designable_ = baseProperty->designable_;
    if (scriptable_ == FlagValueDefault)
        scriptable_ = baseProperty->scriptable_;
    if (writable_ == FlagValueDefault)
        writable_ = baseProperty->writable_;
    if (user_ == FlagValueDefault)
        user_ = baseProperty->user_;
    overrides_ = baseProperty;
}

/*!
 */
QString PropertyNode::qualifiedDataType() const
{
    if (setters().isEmpty() && resetters().isEmpty()) {
        if (type_.contains(QLatin1Char('*')) || type_.contains(QLatin1Char('&'))) {
            // 'QWidget *' becomes 'QWidget *' const
            return type_ + " const";
        }
        else {
            /*
              'int' becomes 'const int' ('int const' is
              correct C++, but looks wrong)
             */
            return "const " + type_;
        }
    }
    else {
        return type_;
    }
}

/*!
  Returns true if this property has an access function named \a name.
 */
bool PropertyNode::hasAccessFunction(const QString &name) const
{
    NodeList::const_iterator i = getters().begin();
    while (i != getters().end()) {
        if ((*i)->name() == name)
            return true;
        ++i;
    }
    i = setters().begin();
    while (i != setters().end()) {
        if ((*i)->name() == name)
            return true;
        ++i;
    }
    i = resetters().begin();
    while (i != resetters().end()) {
        if ((*i)->name() == name)
            return true;
        ++i;
    }
    i = notifiers().begin();
    while (i != notifiers().end()) {
        if ((*i)->name() == name)
            return true;
        ++i;
    }
    return false;
}

bool QmlTypeNode::qmlOnly = false;
QMultiMap<const Node*, Node*> QmlTypeNode::inheritedBy;

/*!
  Constructs a Qml type node or a Js type node depending on
  the value of \a type, which is Node::QmlType by default,
  but which can also be Node::JsType. The new node has the
  given \a parent and \a name.
 */
QmlTypeNode::QmlTypeNode(Aggregate *parent, const QString& name, NodeType type)
    : Aggregate(type, parent, name),
      abstract_(false),
      cnodeRequired_(false),
      wrapper_(false),
      cnode_(nullptr),
      logicalModule_(nullptr),
      qmlBaseNode_(nullptr)
{
    int i = 0;
    if (name.startsWith("QML:")) {
        qDebug() << "BOGUS QML qualifier:" << name;
        i = 4;
    }
    setTitle(name.mid(i));
}

/*!
  Needed for printing a debug messages.
 */
QmlTypeNode::~QmlTypeNode()
{
    // nothing.
}

/*!
  Clear the static maps so that subsequent runs don't try to use
  contents from a previous run.
 */
void QmlTypeNode::terminate()
{
    inheritedBy.clear();
}

/*!
  Record the fact that QML class \a base is inherited by
  QML class \a sub.
 */
void QmlTypeNode::addInheritedBy(const Node *base, Node* sub)
{
    if (sub->isInternal())
        return;
    if (!inheritedBy.contains(base, sub))
        inheritedBy.insert(base, sub);
}

/*!
  Loads the list \a subs with the nodes of all the subclasses of \a base.
 */
void QmlTypeNode::subclasses(const Node *base, NodeList &subs)
{
    subs.clear();
    if (inheritedBy.count(base) > 0) {
        subs = inheritedBy.values(base);
    }
}


/*!
  If this QML type node has a base type node,
  return the fully qualified name of that QML
  type, i.e. <QML-module-name>::<QML-type-name>.
 */
QString QmlTypeNode::qmlFullBaseName() const
{
    QString result;
    if (qmlBaseNode_) {
        result = qmlBaseNode_->logicalModuleName() + "::" + qmlBaseNode_->name();
    }
    return result;
}

/*!
  If the QML type's QML module pointer is set, return the QML
  module name from the QML module node. Otherwise, return the
  empty string.
 */
QString QmlTypeNode::logicalModuleName() const
{
    return (logicalModule_ ? logicalModule_->logicalModuleName() : QString());
}

/*!
  If the QML type's QML module pointer is set, return the QML
  module version from the QML module node. Otherwise, return
  the empty string.
 */
QString QmlTypeNode::logicalModuleVersion() const
{
    return (logicalModule_ ? logicalModule_->logicalModuleVersion() : QString());
}

/*!
  If the QML type's QML module pointer is set, return the QML
  module identifier from the QML module node. Otherwise, return
  the empty string.
 */
QString QmlTypeNode::logicalModuleIdentifier() const
{
    return (logicalModule_ ? logicalModule_->logicalModuleIdentifier() : QString());
}

/*!
  Returns true if this QML type inherits \a type.
 */
bool QmlTypeNode::inherits(Aggregate* type)
{
    QmlTypeNode* qtn = qmlBaseNode();
    while (qtn != nullptr) {
        if (qtn == type)
            return true;
        qtn = qtn->qmlBaseNode();
    }
    return false;
}

/*!
  Constructs either a Qml basic type node or a Javascript
  basic type node, depending on the value pf \a type, which
  must be either Node::QmlBasicType or Node::JsBasicType.
  The new node has the given \a parent and \a name.
 */
QmlBasicTypeNode::QmlBasicTypeNode(Aggregate *parent, const QString& name, Node::NodeType type)
    : Aggregate(type, parent, name)
{
    setTitle(name);
}

/*!
  Constructor for the QML property node.
 */
QmlPropertyNode::QmlPropertyNode(Aggregate* parent,
                                 const QString& name,
                                 const QString& type,
                                 bool attached)
    : Node(parent->isJsType() ? JsProperty : QmlProperty, parent, name),
      type_(type),
      stored_(FlagValueDefault),
      designable_(FlagValueDefault),
      isAlias_(false),
      isdefault_(false),
      attached_(attached),
      readOnly_(FlagValueDefault)
{
    if (type_ == QString("alias"))
        isAlias_ = true;
    if (name.startsWith("__"))
        setStatus(Internal);
}

/*!
  Returns \c true if a QML property or attached property is
  not read-only. The algorithm for figuring this out is long
  amd tedious and almost certainly will break. It currently
  doesn't work for the qmlproperty:

  \code
      bool PropertyChanges::explicit,
  \endcode

  ...because the tokenizer gets confused on \e{explicit}.
 */
bool QmlPropertyNode::isWritable()
{
    if (readOnly_ != FlagValueDefault)
        return !fromFlagValue(readOnly_, false);

    QmlTypeNode* qcn = qmlTypeNode();
    if (qcn) {
        if (qcn->cppClassRequired()) {
            if (qcn->classNode()) {
                PropertyNode* pn = findCorrespondingCppProperty();
                if (pn)
                    return pn->isWritable();
                else
                    defLocation().warning(tr("No Q_PROPERTY for QML property %1::%2::%3 "
                                             "in C++ class documented as QML type: "
                                             "(property not found in the C++ class or its base classes)")
                                          .arg(logicalModuleName()).arg(qmlTypeName()).arg(name()));
            }
            else
                defLocation().warning(tr("No Q_PROPERTY for QML property %1::%2::%3 "
                                         "in C++ class documented as QML type: "
                                         "(C++ class not specified or not found).")
                                      .arg(logicalModuleName()).arg(qmlTypeName()).arg(name()));
        }
    }
    return true;
}

/*!
  Returns a pointer this QML property's corresponding C++
  property, if it has one.
 */
PropertyNode* QmlPropertyNode::findCorrespondingCppProperty()
{
    PropertyNode* pn;
    Node* n = parent();
    while (n && !(n->isQmlType() || n->isJsType()))
        n = n->parent();
    if (n) {
        QmlTypeNode* qcn = static_cast<QmlTypeNode*>(n);
        ClassNode* cn = qcn->classNode();
        if (cn) {
            /*
              If there is a dot in the property name, first
              find the C++ property corresponding to the QML
              property group.
             */
            QStringList dotSplit = name().split(QChar('.'));
            pn = cn->findPropertyNode(dotSplit[0]);
            if (pn) {
                /*
                  Now find the C++ property corresponding to
                  the QML property in the QML property group,
                  <group>.<property>.
                 */
                if (dotSplit.size() > 1) {
                    QStringList path(extractClassName(pn->qualifiedDataType()));
                    Node* nn = QDocDatabase::qdocDB()->findClassNode(path);
                    if (nn) {
                        ClassNode* cn = static_cast<ClassNode*>(nn);
                        PropertyNode *pn2 = cn->findPropertyNode(dotSplit[1]);
                        /*
                          If found, return the C++ property
                          corresponding to the QML property.
                          Otherwise, return the C++ property
                          corresponding to the QML property
                          group.
                         */
                        return (pn2 ? pn2 : pn);
                    }
                }
                else
                    return pn;
            }
        }
    }
    return nullptr;
}

/*!
  Construct the full document name for this node and return it.
 */
QString Node::fullDocumentName() const
{
    QStringList pieces;
    const Node* n = this;

    do {
        if (!n->name().isEmpty())
            pieces.insert(0, n->name());

        if ((n->isQmlType() || n->isJsType()) && !n->logicalModuleName().isEmpty()) {
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
    if (n->isQmlType() || n->isJsType())
        concatenator = QLatin1Char('.');

    if (n->isTextPageNode())
        concatenator = QLatin1Char('#');

    return pieces.join(concatenator);
}

/*!
  Returns the \a str as an NCName, which means the name can
  be used as the value of an \e id attribute. Search for NCName
  on the internet for details of what can be an NCName.
 */
QString Node::cleanId(const QString &str)
{
    QString clean;
    QString name = str.simplified();

    if (name.isEmpty())
        return clean;

    name = name.replace("::","-");
    name = name.replace(QLatin1Char(' '), QLatin1Char('-'));
    name = name.replace("()","-call");

    clean.reserve(name.size() + 20);
    if (!str.startsWith("id-"))
        clean = "id-";
    const QChar c = name[0];
    const uint u = c.unicode();

    if ((u >= 'a' && u <= 'z') ||
            (u >= 'A' && u <= 'Z') ||
            (u >= '0' && u <= '9')) {
        clean += c;
    }
    else if (u == '~') {
        clean += "dtor.";
    }
    else if (u == '_') {
        clean += "underscore.";
    }
    else {
        clean += QLatin1Char('a');
    }

    for (int i = 1; i < (int) name.length(); i++) {
        const QChar c = name[i];
        const uint u = c.unicode();
        if ((u >= 'a' && u <= 'z') ||
                (u >= 'A' && u <= 'Z') ||
                (u >= '0' && u <= '9') || u == '-' ||
                u == '_' || u == '.') {
            clean += c;
        }
        else if (c.isSpace() || u == ':' ) {
            clean += QLatin1Char('-');
        }
        else if (u == '!') {
            clean += "-not";
        }
        else if (u == '&') {
            clean += "-and";
        }
        else if (u == '<') {
            clean += "-lt";
        }
        else if (u == '=') {
            clean += "-eq";
        }
        else if (u == '>') {
            clean += "-gt";
        }
        else if (u == '#') {
            clean += "-hash";
        }
        else if (u == '(') {
            clean += QLatin1Char('-');
        }
        else if (u == ')') {
            clean += QLatin1Char('-');
        }
        else {
            clean += QLatin1Char('-');
            clean += QString::number((int)u, 16);
        }
    }
    return clean;
}

/*!
  Prints the inner node's list of children.
  For debugging only.
 */
void Aggregate::printChildren(const QString& title)
{
    qDebug() << title << name() << children_.size();
    if (children_.size() > 0) {
        for (int i=0; i<children_.size(); ++i) {
            Node* n = children_.at(i);
            qDebug() << "  CHILD:" << n->name() << n->nodeTypeString();
        }
    }
}

/*!
  Removes \a fn from this aggregate's function map. That's
  all it does. If \a fn is in the function map index and it
  has an overload, the value pointer in the function map
  index is set to the the overload pointer. If the function
  has no overload pointer, the function map entry is erased.

  \note When removing a function node from the function map,
  it is important to set the removed function node's next
  overload pointer to null because the function node might
  be added as a child to some other aggregate.
 */
void Aggregate::removeFunctionNode(FunctionNode *fn)
{
    FunctionMap::Iterator i = functionMap_.find(fn->name());
    if (i != functionMap_.end()) {
        if (i.value() == fn) {
            if (fn->nextOverload() != nullptr) {
                i.value() = fn->nextOverload();
                fn->setNextOverload(nullptr);
                fn->setOverloadNumber(0);
            }
            else {
                functionMap_.erase(i);
            }
        } else {
            FunctionNode *current = i.value();
            while (current != nullptr) {
                if (current->nextOverload() == fn) {
                    current->setNextOverload(fn->nextOverload());
                    fn->setNextOverload(nullptr);
                    fn->setOverloadNumber(0);
                    break;
                }
                current = current->nextOverload();
            }
        }
    }
}

/*
  When deciding whether to include a function in the function
  index, if the function is marked private, don't include it.
  If the function is marked obsolete, don't include it. If the
  function is marked internal, don't include it. Or if the
  function is a destructor or any kind of constructor, don't
  include it. Otherwise include it.
 */
static bool keep(FunctionNode *fn)
{
    if (fn->isPrivate() ||
        fn->isObsolete() ||
        fn->isInternal() ||
        fn->isSomeCtor() ||
        fn->isDtor())
        return false;
    return true;
}

/*!
  Insert all functions declared in this aggregate into the
  \a functionIndex. Call the function recursively for each
  child that is an aggregate.

  Only include functions that are in the public API and
  that are not constructors or destructors.
 */
void Aggregate::findAllFunctions(NodeMapMap &functionIndex)
{
    FunctionMap::const_iterator i;
    for (i = functionMap_.constBegin(); i != functionMap_.constEnd(); ++i) {
        FunctionNode *fn = i.value();
        if (keep(fn))
            functionIndex[fn->name()].insert(fn->parent()->fullDocumentName(), fn);
        fn = fn->nextOverload();
        while (fn != nullptr) {
            if (keep(fn))
                functionIndex[fn->name()].insert(fn->parent()->fullDocumentName(), fn);
            fn = fn->nextOverload();
        }
    }
    foreach (Node *n, children_) {
        if (n->isAggregate() && !n->isPrivate())
            static_cast<Aggregate*>(n)->findAllFunctions(functionIndex);
    }
}

/*!
  For each child of this node, if the child is a namespace node,
  insert the child into the \a namespaces multimap. If the child
  is an aggregate, call this function recursively for that child.

  When the function called with the root node of a tree, it finds
  all the namespace nodes in that tree and inserts them into the
  \a namespaces multimap.

  The root node of a tree is a namespace, but it has no name, so
  it is not inserted into the map. So, if this function is called
  for each tree in the qdoc database, it finds all the namespace
  nodes in the database.
  */
void Aggregate::findAllNamespaces(NodeMultiMap &namespaces)
{
    foreach (Node *n, children_) {
        if (n->isAggregate() && !n->isPrivate()) {
            if (n->isNamespace() && !n->name().isEmpty())
                namespaces.insert(n->name(), n);
            static_cast<Aggregate*>(n)->findAllNamespaces(namespaces);
        }
    }
}

/*!
  Returns true if this aggregate contains at least one child
  that is marked obsolete. Otherwise returns false.
 */
bool Aggregate::hasObsoleteMembers()
{
    foreach (Node *n, children_) {
        if (!n->isPrivate() && n->isObsolete()) {
            if (n->isFunction() || n->isProperty() || n->isEnumType() ||
                n->isTypedef() || n->isTypeAlias() || n->isVariable() ||
                n->isQmlProperty() || n->isJsProperty())
                return true;
        }
    }
    return false;
}

/*!
  Finds all the obsolete C++ classes and QML types in this
  aggregate and all the C++ classes and QML types with obsolete
  members, and inserts them into maps used elesewhere for
  generating documentation.
 */
void Aggregate::findAllObsoleteThings()
{
    foreach (Node *n, children_) {
        if (!n->isPrivate()) {
            QString name = n->name();
            if (n->isObsolete()) {
                if (n->isClassNode())
                    QDocDatabase::obsoleteClasses().insert(n->qualifyCppName(), n);
                else if (n->isQmlType() || n->isJsType())
                    QDocDatabase::obsoleteQmlTypes().insert(n->qualifyQmlName(), n);
            } else if (n->isClassNode()) {
                Aggregate* a = static_cast<Aggregate*>(n);
                if (a->hasObsoleteMembers())
                    QDocDatabase::classesWithObsoleteMembers().insert(n->qualifyCppName(), n);
            }
            else if (n->isQmlType() || n->isJsType()) {
                Aggregate *a = static_cast<Aggregate *>(n);
                if (a->hasObsoleteMembers())
                    QDocDatabase::qmlTypesWithObsoleteMembers().insert(n->qualifyQmlName(), n);
            }
            else if (n->isAggregate()) {
                static_cast<Aggregate*>(n)->findAllObsoleteThings();
            }
        }
    }
}

/*!
  Finds all the C++ classes, QML types, JS types, QML and JS
  basic types, and examples in this aggregate and inserts them
  into appropriate maps for later use in generating documentation.
 */
void Aggregate::findAllClasses()
{
    foreach (Node *n, children_) {
        if (!n->isPrivate() && !n->isInternal() && !n->isDontDocument() &&
            n->tree()->camelCaseModuleName() != QString("QDoc")) {
            if (n->isClassNode()) {
                QDocDatabase::cppClasses().insert(n->qualifyCppName().toLower(), n);
            } else if (n->isQmlType() || n->isQmlBasicType() || n->isJsType() || n->isJsBasicType()) {
                QString name = n->unqualifyQmlName();
                QDocDatabase::qmlTypes().insert(name, n);
                //also add to the QML basic type map
                if (n->isQmlBasicType() || n->isJsBasicType())
                    QDocDatabase::qmlBasicTypes().insert(name, n);
            } else if (n->isExample()) {
                // use the module index title as key for the example map
                QString title = n->tree()->indexTitle();
                if (!QDocDatabase::examples().contains(title, n))
                    QDocDatabase::examples().insert(title, n);
            } else if (n->isAggregate()) {
                static_cast<Aggregate*>(n)->findAllClasses();
            }
        }
    }
}

/*!
  Find all the attribution pages in this node and insert them
  into \a attributions.
 */
void Aggregate::findAllAttributions(NodeMultiMap &attributions)
{
    foreach (Node *n, children_) {
        if (!n->isPrivate()) {
            if (n->pageType() == Node::AttributionPage)
                attributions.insertMulti(n->tree()->indexTitle(), n);
            else if (n->isAggregate())
                static_cast<Aggregate*>(n)->findAllAttributions(attributions);
        }
    }
}

/*!
  \fn void findAllSince()

  Finds all the nodes in this node where a \e{since} command appeared
  in the qdoc comment and sorts them into maps according to the kind
  of node.

  This function is used for generating the "New Classes... in x.y"
  section on the \e{What's New in Qt x.y} page.
 */
void Aggregate::findAllSince()
{
    foreach (Node *n, children_) {
        QString sinceString = n->since();
        // Insert a new entry into each map for each new since string found.
        if (!n->isPrivate() && !sinceString.isEmpty()) {
            NodeMultiMapMap::iterator nsmap = QDocDatabase::newSinceMaps().find(sinceString);
            if (nsmap == QDocDatabase::newSinceMaps().end())
                nsmap = QDocDatabase::newSinceMaps().insert(sinceString, NodeMultiMap());

            NodeMapMap::iterator ncmap = QDocDatabase::newClassMaps().find(sinceString);
            if (ncmap == QDocDatabase::newClassMaps().end())
                ncmap = QDocDatabase::newClassMaps().insert(sinceString, NodeMap());

            NodeMapMap::iterator nqcmap = QDocDatabase::newQmlTypeMaps().find(sinceString);
            if (nqcmap == QDocDatabase::newQmlTypeMaps().end())
                nqcmap = QDocDatabase::newQmlTypeMaps().insert(sinceString, NodeMap());

            if (n->isFunction()) {
                // Insert functions into the general since map.
                FunctionNode *fn = static_cast<FunctionNode*>(n);
                if (!fn->isObsolete() && !fn->isSomeCtor() && !fn->isDtor())
                    nsmap.value().insert(fn->name(), fn);
            }
            else if (n->isClassNode()) {
                // Insert classes into the since and class maps.
                QString name = n->qualifyWithParentName();
                nsmap.value().insert(name, n);
                ncmap.value().insert(name, n);
            } else if (n->isQmlType() || n->isJsType()) {
                // Insert QML elements into the since and element maps.
                QString name = n->qualifyWithParentName();
                nsmap.value().insert(name, n);
                nqcmap.value().insert(name, n);
            } else if (n->isQmlProperty() || n->isJsProperty()) {
                // Insert QML properties into the since map.
                nsmap.value().insert(n->name(), n);
            } else {
                // Insert external documents into the general since map.
                QString name = n->qualifyWithParentName();
                nsmap.value().insert(name, n);
            }
        }
        // Recursively find child nodes with since commands.
        if (n->isAggregate())
            static_cast<Aggregate*>(n)->findAllSince();
    }
}

/*!
  For each QML Type node in this aggregate's children, if the
  QML type has a QML base type name but its QML base type node
  pointer is nullptr, use the QML base type name to look up the
  base type node. If the node is found, set the node's QML base
  type node pointer to that node.
 */
void Aggregate::resolveQmlInheritance()
{
    NodeMap previousSearches;
    // Do we need recursion?
    foreach (Node *child, children_) {
        if (!child->isQmlType() && !child->isJsType())
            continue;
        QmlTypeNode *type = static_cast<QmlTypeNode *>(child);
        if (type->qmlBaseNode() != nullptr)
            continue;
        if (type->qmlBaseName().isEmpty())
            continue;
        QmlTypeNode *base = static_cast<QmlTypeNode *>(previousSearches.value(type->qmlBaseName()));
        if (base && (base != type)) {
            type->setQmlBaseNode(base);
            QmlTypeNode::addInheritedBy(base, type);
        } else {
            if (!type->importList().isEmpty()) {
                const ImportList &imports = type->importList();
                for (int i=0; i<imports.size(); ++i) {
                    base = QDocDatabase::qdocDB()->findQmlType(imports[i], type->qmlBaseName());
                    if (base && (base != type)) {
                        if (base->logicalModuleVersion()[0] != imports[i].version_[0])
                            base = nullptr; // Safeguard for QTBUG-53529
                        break;
                    }
                }
            }
            if (base == nullptr) {
                base = QDocDatabase::qdocDB()->findQmlType(QString(), type->qmlBaseName());
            }
            if (base && (base != type)) {
                type->setQmlBaseNode(base);
                QmlTypeNode::addInheritedBy(base, type);
                previousSearches.insert(type->qmlBaseName(), base);
            }
        }
    }
}

/*!
  Returns a word representing the kind of Aggregate this node is.
  Currently only works for class, struct, and union, but it can
  easily be extended. If \a cap is true, the word is capitalised.
 */
QString Aggregate::typeWord(bool cap) const
{
    if (cap) {
        switch (nodeType()) {
        case Node::Class:
            return QLatin1String("Class");
        case Node::Struct:
            return QLatin1String("Struct");
        case Node::Union:
            return QLatin1String("Union");
        default:
            break;
        }
    } else {
        switch (nodeType()) {
        case Node::Class:
            return QLatin1String("class");
        case Node::Struct:
            return QLatin1String("struct");
        case Node::Union:
            return QLatin1String("union");
        default:
            break;
        }
    }
    return QString();
}

/*!
  \class ProxyNode
  \brief A class for representing an Aggregate that is documented in a different module.

  This class is used to represent an Aggregate (usually a class)
  that is located and documented in a different module. In the
  current module, a ProxyNode holds child nodes that are related
  to the class in the other module.

  For example, class QHash is located and documented in QtCore.
  There are many global functions named qHash() in QtCore that
  are all related to class QHash using the \c relates command.
  There are also a few qHash() function in QtNetwork that are
  related to QHash. These functions must be documented when the
  documentation for QtNetwork is generated, but the reference
  page for QHash must link to that documentation in its related
  nonmembers list.

  The ProxyNode allows qdoc to construct links to the related
  functions (or other things?) in QtNetwork from the reference
  page in QtCore.
 */

/*!
  Constructs the ProxyNode, which at this point looks like any
  other Aggregate, and then finds the Tree this node is in and
  appends this node to that Tree's proxy list so it will be
  easy to find later.
 */
ProxyNode::ProxyNode(Aggregate *parent, const QString &name)
    : Aggregate(Node::Proxy, parent, name)
{
    tree()->appendProxy(this);
}

/*!
  \class CollectionNode
  \brief A class for holding the members of a collection of doc pages.
 */

/*!
  Returns \c true if the collection node's member list is
  not empty.
 */
bool CollectionNode::hasMembers() const
{
    return !members_.isEmpty();
}

/*!
  Appends \a node to the collection node's member list, if
  and only if it isn't already in the member list.
 */
void CollectionNode::addMember(Node* node)
{
    if (!members_.contains(node))
        members_.append(node);
}

/*!
  Returns \c true if this collection node contains at least
  one namespace node.
 */
bool CollectionNode::hasNamespaces() const
{
    if (!members_.isEmpty()) {
        NodeList::const_iterator i = members_.begin();
        while (i != members_.end()) {
            if ((*i)->isNamespace())
                return true;
            ++i;
        }
    }
    return false;
}

/*!
  Returns \c true if this collection node contains at least
  one class node.
 */
bool CollectionNode::hasClasses() const
{
    if (!members_.isEmpty()) {
        NodeList::const_iterator i = members_.cbegin();
        while (i != members_.cend()) {
            if ((*i)->isClassNode())
                return true;
            ++i;
        }
    }
    return false;
}

/*!
  Loads \a out with all this collection node's members that
  are namespace nodes.
 */
void CollectionNode::getMemberNamespaces(NodeMap& out)
{
    out.clear();
    NodeList::const_iterator i = members_.cbegin();
    while (i != members_.cend()) {
        if ((*i)->isNamespace())
            out.insert((*i)->name(),(*i));
        ++i;
    }
}

/*!
  Loads \a out with all this collection node's members that
  are class nodes.
 */
void CollectionNode::getMemberClasses(NodeMap& out) const
{
    out.clear();
    NodeList::const_iterator i = members_.cbegin();
    while (i != members_.cend()) {
        if ((*i)->isClassNode())
            out.insert((*i)->name(),(*i));
        ++i;
    }
}

/*!
  Prints the collection node's list of members.
  For debugging only.
 */
void CollectionNode::printMembers(const QString& title)
{
    qDebug() << title << name() << members_.size();
    if (members_.size() > 0) {
        for (int i=0; i<members_.size(); ++i) {
            Node* n = members_.at(i);
            qDebug() << "  MEMBER:" << n->name() << n->nodeTypeString();
        }
    }
}

/*!
  This function splits \a arg on the blank character to get a
  logical module name and version number. If the version number
  is present, it spilts the version number on the '.' character
  to get a major version number and a minor vrsion number. If
  the version number is present, both the major and minor version
  numbers should be there, but the minor version number is not
  absolutely necessary.
 */
void CollectionNode::setLogicalModuleInfo(const QString& arg)
{
    QStringList blankSplit = arg.split(QLatin1Char(' '));
    logicalModuleName_ = blankSplit[0];
    if (blankSplit.size() > 1) {
        QStringList dotSplit = blankSplit[1].split(QLatin1Char('.'));
        logicalModuleVersionMajor_ = dotSplit[0];
        if (dotSplit.size() > 1)
            logicalModuleVersionMinor_ = dotSplit[1];
        else
            logicalModuleVersionMinor_ = "0";
    }
}

/*!
  This function accepts the logical module \a info as a string
  list. If the logical module info contains the version number,
  it spilts the version number on the '.' character to get the
  major and minor vrsion numbers. Both major and minor version
  numbers should be provided, but the minor version number is
  not strictly necessary.
 */
void CollectionNode::setLogicalModuleInfo(const QStringList& info)
{
    logicalModuleName_ = info[0];
    if (info.size() > 1) {
        QStringList dotSplit = info[1].split(QLatin1Char('.'));
        logicalModuleVersionMajor_ = dotSplit[0];
        if (dotSplit.size() > 1)
            logicalModuleVersionMinor_ = dotSplit[1];
        else
            logicalModuleVersionMinor_ = "0";
    }
}

/*!
  Searches the shared comment node's member nodes for function
  nodes. Each function node's overload flag is set.
 */
void SharedCommentNode::setOverloadFlags()
{
    for (Node *n : collective_) {
        if (n->isFunction())
            static_cast<FunctionNode*>(n)->setOverloadFlag();
    }
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent. Return the pointer to the clone.
 */
Node *SharedCommentNode::clone(Aggregate *parent)
{
    SharedCommentNode *scn = new SharedCommentNode(*this); // shallow copy
    scn->setParent(nullptr);
    parent->addChild(scn);
    return scn;
}


/*!
  Sets the related nonmember flag in this node and in each
  node in the shared comment's collective.
 */
void SharedCommentNode::setRelatedNonmember(bool b)
{
    Node::setRelatedNonmember(b);
    for (Node *n : collective_)
        n->setRelatedNonmember(b);
}

QT_END_NAMESPACE
