// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "aggregate.h"

#include "functionnode.h"
#include "parameters.h"
#include "typedefnode.h"
#include "qdocdatabase.h"
#include "qmlpropertynode.h"
#include "qmltypenode.h"
#include "sharedcommentnode.h"

QT_BEGIN_NAMESPACE

/*!
  \class Aggregate
 */

/*! \fn Aggregate::Aggregate(NodeType type, Aggregate *parent, const QString &name)
  The constructor should never be called directly. It is only called
  by the constructors of subclasses of Aggregate. Those constructors
  pass the node \a type they want to create, the \a parent of the new
  node, and its \a name.
 */

/*!
  Recursively set all non-related members in the list of children to
  \nullptr, after which each aggregate can safely delete all children
  in their list. Aggregate's destructor calls this only on the root
  namespace node.
 */
void Aggregate::dropNonRelatedMembers()
{
    for (auto &child : m_children) {
        if (!child)
            continue;
        if (child->parent() != this)
            child = nullptr;
        else if (child->isAggregate())
            static_cast<Aggregate*>(child)->dropNonRelatedMembers();
    }
}

/*!
  Destroys this Aggregate; deletes each child.
 */
Aggregate::~Aggregate()
{
    // If this is the root, clear non-related children first
    if (isNamespace() && name().isEmpty())
        dropNonRelatedMembers();

    m_enumChildren.clear();
    m_nonfunctionMap.clear();
    m_functionMap.clear();
    qDeleteAll(m_children.begin(), m_children.end());
    m_children.clear();
}

/*!
  If \a genus is \c{Node::DontCare}, find the first node in
  this node's child list that has the given \a name. If this
  node is a QML type, be sure to also look in the children
  of its property group nodes. Return the matching node or \c nullptr.

  If \a genus is either \c{Node::CPP} or \c {Node::QML}, then
  find all this node's children that have the given \a name,
  and return the one that satisfies the \a genus requirement.
 */
Node *Aggregate::findChildNode(const QString &name, Node::Genus genus, int findFlags) const
{
    if (genus == Node::DontCare) {
        Node *node = m_nonfunctionMap.value(name);
        if (node)
            return node;
    } else {
        const NodeList &nodes = m_nonfunctionMap.values(name);
        for (auto *node : nodes) {
            if (genus & node->genus()) {
                if (findFlags & TypesOnly) {
                    if (!node->isTypedef() && !node->isClassNode()
                        && !node->isQmlType() && !node->isEnumType())
                        continue;
                } else if (findFlags & IgnoreModules && node->isModule())
                    continue;
                return node;
            }
        }
    }
    if (genus != Node::DontCare && !(genus & this->genus()))
        return nullptr;
    return m_functionMap.value(name);
}

/*!
  Find all the child nodes of this node that are named
  \a name and return them in \a nodes.
 */
void Aggregate::findChildren(const QString &name, NodeVector &nodes) const
{
    nodes.clear();
    int nonfunctionCount = m_nonfunctionMap.count(name);
    auto it = m_functionMap.find(name);
    if (it != m_functionMap.end()) {
        int functionCount = 0;
        FunctionNode *fn = it.value();
        while (fn != nullptr) {
            ++functionCount;
            fn = fn->nextOverload();
        }
        nodes.reserve(nonfunctionCount + functionCount);
        fn = it.value();
        while (fn != nullptr) {
            nodes.append(fn);
            fn = fn->nextOverload();
        }
    } else {
        nodes.reserve(nonfunctionCount);
    }
    if (nonfunctionCount > 0) {
        for (auto it = m_nonfunctionMap.find(name);
             it != m_nonfunctionMap.end() && it.key() == name; ++it) {
            nodes.append(it.value());
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
Node *Aggregate::findNonfunctionChild(const QString &name, bool (Node::*isMatch)() const)
{
    const NodeList &nodes = m_nonfunctionMap.values(name);
    for (auto *node : nodes) {
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
    auto it = m_functionMap.find(name);
    if (it == m_functionMap.end())
        return nullptr;
    FunctionNode *fn = it.value();

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
        for (fn = it.value(); fn != nullptr; fn = fn->nextOverload())
            if (!fn->isInternal())
                return fn;
        return it.value();
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
    FunctionNode *fn = m_functionMap.value(clone->name());
    while (fn != nullptr) {
        if (isSameSignature(clone, fn))
            return fn;
        fn = fn->nextOverload();
    }
    return nullptr;
}

/*!
  Mark all child nodes that have no documentation as having
  private access and internal status. qdoc will then ignore
  them for documentation purposes.
 */
void Aggregate::markUndocumentedChildrenInternal()
{
    for (auto *child : std::as_const(m_children)) {
        if (!child->isSharingComment() && !child->hasDoc() && !child->isDontDocument()) {
            if (!child->docMustBeGenerated()) {
                if (child->isFunction()) {
                    if (static_cast<FunctionNode *>(child)->hasAssociatedProperties())
                        continue;
                } else if (child->isTypedef()) {
                    if (static_cast<TypedefNode *>(child)->hasAssociatedEnum())
                        continue;
                }
                child->setAccess(Access::Private);
                child->setStatus(Node::Internal);
            }
        }
        if (child->isAggregate()) {
            static_cast<Aggregate *>(child)->markUndocumentedChildrenInternal();
        }
    }
}

/*!
  This is where we set the overload numbers for function nodes.
 */
void Aggregate::normalizeOverloads()
{
    /*
      Ensure that none of the primary functions is inactive, private,
      or marked \e {overload}.
    */
    for (auto it = m_functionMap.begin(); it != m_functionMap.end(); ++it) {
        FunctionNode *fn = it.value();
        if (fn->isOverload()) {
            FunctionNode *primary = fn->findPrimaryFunction();
            if (primary) {
                primary->setNextOverload(fn);
                it.value() = primary;
                fn = primary;
            }
        }
        int count = 0;
        fn->setOverloadNumber(0); // also clears the overload flag
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
                    fn = fn->nextOverload();
                }
            } else {
                fn->setNextOverload(internalFn);
                break;
            }
        }
        while (internalFn) {
            internalFn->setOverloadNumber(++count);
            internalFn = internalFn->nextOverload();
        }
    }

    for (auto *node : std::as_const(m_children)) {
        if (node->isAggregate())
            static_cast<Aggregate *>(node)->normalizeOverloads();
    }
}

/*!
  Returns a const reference to the list of child nodes of this
  aggregate that are not function nodes. Duplicate nodes are
  removed from the list.
 */
const NodeList &Aggregate::nonfunctionList()
{
    m_nonfunctionList = m_nonfunctionMap.values();
    std::sort(m_nonfunctionList.begin(), m_nonfunctionList.end(), Node::nodeNameLessThan);
    m_nonfunctionList.erase(std::unique(m_nonfunctionList.begin(), m_nonfunctionList.end()),
                            m_nonfunctionList.end());
    return m_nonfunctionList;
}

/*! \fn bool Aggregate::isAggregate() const
  Returns \c true because this node is an instance of Aggregate,
  which means it can have children.
 */

/*!
  Finds the enum type node that has \a enumValue as one of
  its enum values and returns a pointer to it. Returns 0 if
  no enum type node is found that has \a enumValue as one
  of its values.
 */
const EnumNode *Aggregate::findEnumNodeForValue(const QString &enumValue) const
{
    for (const auto *node : m_enumChildren) {
        const auto *en = static_cast<const EnumNode *>(node);
        if (en->hasItem(enumValue))
            return en;
    }
    return nullptr;
}

/*!
  Compare \a f1 to \a f2 and return \c true if they have the same
  signature. Otherwise return \c false. They must have the same
  number of parameters, and all the parameter types must be the
  same. The functions must have the same constness and refness.
  This is a private function.
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

            if (t1.size() < t2.size())
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
  but are in a linked list using the FunctionNode's m_nextOverload
  pointer.

  \note The function's overload number and overload flag are set in
  normalizeOverloads().

  \note This is a private function.

  \sa normalizeOverloads()
 */
void Aggregate::addFunction(FunctionNode *fn)
{
    auto it = m_functionMap.find(fn->name());
    if (it == m_functionMap.end())
        m_functionMap.insert(fn->name(), fn);
    else
        it.value()->appendOverload(fn);
}

/*!
  When an Aggregate adopts a function \a fn that is a child of
  another Aggregate, the function is inserted into this
  Aggregate's function map.

  The function is also removed from the overload list
  that's relative to the original parent \a firstParent.

  \note This is a private function.
 */
void Aggregate::adoptFunction(FunctionNode *fn, Aggregate *firstParent)
{
    auto *primary = firstParent->m_functionMap.value(fn->name());
    if (primary) {
        if (primary != fn)
            primary->removeOverload(fn);
        else if (primary->nextOverload())
            firstParent->m_functionMap.insert(primary->name(),
                                              primary->nextOverload());
        /* else...technically we should call
        firstParent->m_functionMap.remove(primary->name());
        but we want to be able to still find global functions
        from the global namespace, even after adopting them
        elsewhere.
        */
    }
    fn->setNextOverload(nullptr);
    addFunction(fn);
}

/*!
  Adds the \a child to this node's child map using \a title
  as the key. The \a child is not added to the child list
  again, because it is presumed to already be there. We just
  want to be able to find the child by its \a title.
 */
void Aggregate::addChildByTitle(Node *child, const QString &title)
{
    m_nonfunctionMap.insert(title, child);
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
    m_children.append(child);
    child->setParent(this);
    child->setOutputSubdirectory(this->outputSubdirectory());
    child->setUrl(QString());
    child->setIndexNodeFlag(isIndexNode());

    if (child->isFunction()) {
        addFunction(static_cast<FunctionNode *>(child));
    } else if (!child->name().isEmpty()) {
        m_nonfunctionMap.insert(child->name(), child);
        if (child->isEnumType())
            m_enumChildren.append(child);
    }
}

/*!
  This Aggregate becomes the adoptive parent of \a child. The
  \a child knows this Aggregate as its parent, but its former
  parent continues to have pointers to the child in its child
  list and in its searchable data structures. But the child is
  also added to the child list and searchable data structures
  of this Aggregate.
 */
void Aggregate::adoptChild(Node *child)
{
    if (child->parent() != this) {
        m_children.append(child);
        auto firstParent = child->parent();
        child->setParent(this);
        if (child->isFunction()) {
            adoptFunction(static_cast<FunctionNode *>(child), firstParent);
        } else if (!child->name().isEmpty()) {
            m_nonfunctionMap.insert(child->name(), child);
            if (child->isEnumType())
                m_enumChildren.append(child);
        }
        if (child->isSharedCommentNode()) {
            auto *scn = static_cast<SharedCommentNode *>(child);
            for (Node *n : scn->collective())
                adoptChild(n);
        }
    }
}

/*!
  Recursively sets the output subdirectory for children
 */
void Aggregate::setOutputSubdirectory(const QString &t)
{
    Node::setOutputSubdirectory(t);
    for (auto *node : std::as_const(m_children))
        node->setOutputSubdirectory(t);
}

/*!
  If this node has a child that is a QML property named \a n, return a
  pointer to that child. Otherwise, return \nullptr.
 */
QmlPropertyNode *Aggregate::hasQmlProperty(const QString &n) const
{
    NodeType goal = Node::QmlProperty;
    for (auto *child : std::as_const(m_children)) {
        if (child->nodeType() == goal) {
            if (child->name() == n)
                return static_cast<QmlPropertyNode *>(child);
        }
    }
    return nullptr;
}

/*!
  If this node has a child that is a QML property named \a n and that
  also matches \a attached, return a pointer to that child.
 */
QmlPropertyNode *Aggregate::hasQmlProperty(const QString &n, bool attached) const
{
    NodeType goal = Node::QmlProperty;
    for (auto *child : std::as_const(m_children)) {
        if (child->nodeType() == goal) {
            if (child->name() == n && child->isAttached() == attached)
                return static_cast<QmlPropertyNode *>(child);
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
    auto it = m_functionMap.find(fn->name());
    return !(it == m_functionMap.end()) && (it.value()->nextOverload() != nullptr);
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
    if (fn->isPrivate() || fn->isDeprecated() || fn->isInternal() || fn->isSomeCtor() || fn->isDtor())
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
    for (auto it = m_functionMap.constBegin(); it != m_functionMap.constEnd(); ++it) {
        FunctionNode *fn = it.value();
        if (keep(fn))
            functionIndex[fn->name()].insert(fn->parent()->fullDocumentName(), fn);
        fn = fn->nextOverload();
        while (fn != nullptr) {
            if (keep(fn))
                functionIndex[fn->name()].insert(fn->parent()->fullDocumentName(), fn);
            fn = fn->nextOverload();
        }
    }
    for (Node *node : std::as_const(m_children)) {
        if (node->isAggregate() && !node->isPrivate() && !node->isDontDocument())
            static_cast<Aggregate *>(node)->findAllFunctions(functionIndex);
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
    for (auto *node : std::as_const(m_children)) {
        if (node->isAggregate() && !node->isPrivate()) {
            if (node->isNamespace() && !node->name().isEmpty())
                namespaces.insert(node->name(), node);
            static_cast<Aggregate *>(node)->findAllNamespaces(namespaces);
        }
    }
}

/*!
  Returns true if this aggregate contains at least one child
  that is marked obsolete. Otherwise returns false.
 */
bool Aggregate::hasObsoleteMembers() const
{
    for (const auto *node : m_children)
        if (!node->isPrivate() && node->isDeprecated()) {
            if (node->isFunction() || node->isProperty() || node->isEnumType() || node->isTypedef()
                || node->isTypeAlias() || node->isVariable() || node->isQmlProperty())
                return true;
        }
    return false;
}

/*!
  Finds all the obsolete C++ classes and QML types in this
  aggregate and all the C++ classes and QML types with obsolete
  members, and inserts them into maps used elsewhere for
  generating documentation.
 */
void Aggregate::findAllObsoleteThings()
{
    for (auto *node : std::as_const(m_children)) {
        if (!node->isPrivate()) {
            if (node->isDeprecated()) {
                if (node->isClassNode())
                    QDocDatabase::obsoleteClasses().insert(node->qualifyCppName(), node);
                else if (node->isQmlType())
                    QDocDatabase::obsoleteQmlTypes().insert(node->qualifyQmlName(), node);
            } else if (node->isClassNode()) {
                auto *a = static_cast<Aggregate *>(node);
                if (a->hasObsoleteMembers())
                    QDocDatabase::classesWithObsoleteMembers().insert(node->qualifyCppName(), node);
            } else if (node->isQmlType()) {
                auto *a = static_cast<Aggregate *>(node);
                if (a->hasObsoleteMembers())
                    QDocDatabase::qmlTypesWithObsoleteMembers().insert(node->qualifyQmlName(),
                                                                       node);
            } else if (node->isAggregate()) {
                static_cast<Aggregate *>(node)->findAllObsoleteThings();
            }
        }
    }
}

/*!
  Finds all the C++ classes, QML types, QML basic types, and examples
  in this aggregate and inserts them into appropriate maps for later
  use in generating documentation.
 */
void Aggregate::findAllClasses()
{
    for (auto *node : std::as_const(m_children)) {
        if (!node->isPrivate() && !node->isInternal() && !node->isDontDocument()
            && node->tree()->camelCaseModuleName() != QString("QDoc")) {
            if (node->isClassNode()) {
                QDocDatabase::cppClasses().insert(node->qualifyCppName().toLower(), node);
            } else if (node->isQmlType()) {
                QString name = node->name().toLower();
                QDocDatabase::qmlTypes().insert(name, node);
                // also add to the QML basic type map
                if (node->isQmlBasicType())
                    QDocDatabase::qmlBasicTypes().insert(name, node);
            } else if (node->isExample()) {
                // use the module index title as key for the example map
                QString title = node->tree()->indexTitle();
                if (!QDocDatabase::examples().contains(title, node))
                    QDocDatabase::examples().insert(title, node);
            } else if (node->isAggregate()) {
                static_cast<Aggregate *>(node)->findAllClasses();
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
    for (auto *node : std::as_const(m_children)) {
        if (!node->isPrivate()) {
            if (node->isPageNode() && static_cast<PageNode*>(node)->isAttribution())
                attributions.insert(node->tree()->indexTitle(), node);
            else if (node->isAggregate())
                static_cast<Aggregate *>(node)->findAllAttributions(attributions);
        }
    }
}

/*!
  Finds all the nodes in this node where a \e{since} command appeared
  in the qdoc comment and sorts them into maps according to the kind
  of node.

  This function is used for generating the "New Classes... in x.y"
  section on the \e{What's New in Qt x.y} page.
 */
void Aggregate::findAllSince()
{
    for (auto *node : std::as_const(m_children)) {
        if (node->isRelatedNonmember() && node->parent() != this)
            continue;
        QString sinceString = node->since();
        // Insert a new entry into each map for each new since string found.
        if (node->isInAPI() && !sinceString.isEmpty()) {
            // operator[] will insert a default-constructed value into the
            // map if key is not found, which is what we want here.
            auto &nsmap = QDocDatabase::newSinceMaps()[sinceString];
            auto &ncmap = QDocDatabase::newClassMaps()[sinceString];
            auto &nqcmap = QDocDatabase::newQmlTypeMaps()[sinceString];

            if (node->isFunction()) {
                // Insert functions into the general since map.
                auto *fn = static_cast<FunctionNode *>(node);
                if (!fn->isDeprecated() && !fn->isSomeCtor() && !fn->isDtor())
                    nsmap.insert(fn->name(), fn);
            } else if (node->isClassNode()) {
                // Insert classes into the since and class maps.
                QString name = node->qualifyWithParentName();
                nsmap.insert(name, node);
                ncmap.insert(name, node);
            } else if (node->isQmlType()) {
                // Insert QML elements into the since and element maps.
                QString name = node->qualifyWithParentName();
                nsmap.insert(name, node);
                nqcmap.insert(name, node);
            } else if (node->isQmlProperty()) {
                // Insert QML properties into the since map.
                nsmap.insert(node->name(), node);
            } else {
                // Insert external documents into the general since map.
                QString name = node->qualifyWithParentName();
                nsmap.insert(name, node);
            }
        }
        // Enum values - a special case as EnumItem is not a Node subclass
        if (node->isInAPI() && node->isEnumType()) {
            for (const auto &val : static_cast<EnumNode *>(node)->items()) {
                sinceString = val.since();
                if (sinceString.isEmpty())
                    continue;
                // Insert to enum value map
                QDocDatabase::newEnumValueMaps()[sinceString].insert(
                        node->name() + "::" + val.name(), node);
                // Ugly hack: Insert into general map with an empty key -
                // we need something in there to mark the corresponding
                // section populated. See Sections class constructor.
                QDocDatabase::newSinceMaps()[sinceString].replace(QString(), node);
            }
        }

        // Recursively find child nodes with since commands.
        if (node->isAggregate())
            static_cast<Aggregate *>(node)->findAllSince();
    }
}

/*!
  Resolves the inheritance information for all QML type children
  of this aggregate.
*/
void Aggregate::resolveQmlInheritance()
{
    NodeMap previousSearches;
    for (auto *child : std::as_const(m_children)) {
        if (!child->isQmlType())
            continue;
        static_cast<QmlTypeNode *>(child)->resolveInheritance(previousSearches);
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

/*! \fn int Aggregate::count() const
  Returns the number of children in the child list.
 */

/*! \fn const NodeList &Aggregate::childNodes() const
  Returns a const reference to the child list.
 */

/*! \fn NodeList::ConstIterator Aggregate::constBegin() const
  Returns a const iterator pointing at the beginning of the child list.
 */

/*! \fn NodeList::ConstIterator Aggregate::constEnd() const
  Returns a const iterator pointing at the end of the child list.
 */

/*! \fn QmlTypeNode *Aggregate::qmlBaseNode() const
  If this Aggregate is a QmlTypeNode, this function returns a pointer to
  the QmlTypeNode that is its base type. Otherwise it returns \c nullptr.
  A QmlTypeNode doesn't always have a base type, so even when this Aggregate
  is aQmlTypeNode, the pointer returned can be \c nullptr.
 */

/*! \fn FunctionMap &Aggregate::functionMap()
  Returns a reference to this Aggregate's function map, which
  is a map of all the children of this Aggregate that are
  FunctionNodes.
 */

/*! \fn void Aggregate::appendToRelatedByProxy(const NodeList &t)
  Appends the list of node pointers to the list of elements that are
  related to this Aggregate but are documented in a different module.

  \sa relatedByProxy()
 */

/*! \fn NodeList &Aggregate::relatedByProxy()
  Returns a reference to a list of node pointers where each element
  points to a node in an index file for some other module, such that
  whatever the node represents was documented in that other module,
  but it is related to this Aggregate, so when the documentation for
  this Aggregate is written, it will contain links to elements in the
  other module.
 */

QT_END_NAMESPACE
