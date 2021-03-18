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

#include "tree.h"

#include "doc.h"
#include "htmlgenerator.h"
#include "location.h"
#include "node.h"
#include "qdocdatabase.h"
#include "text.h"

#include <QtCore/qdebug.h>

#include <limits.h>

QT_BEGIN_NAMESPACE

/*!
  \class Tree

  This class constructs and maintains a tree of instances of
  the subclasses of Node.

  This class is now private. Only class QDocDatabase has access.
  Please don't change this. If you must access class Tree, do it
  though the pointer to the singleton QDocDatabase.

  Tree is being converted to a forest. A static member provides a
  map of Tree *values with the module names as the keys. There is
  one Tree in the map for each index file read, and there is one
  tree that is not in the map for the module whose documentation
  is being generated.
 */

/*!
  Constructs a Tree. \a qdb is the pointer to the singleton
  qdoc database that is constructing the tree. This might not
  be necessary, and it might be removed later.

  \a camelCaseModuleName is the project name for this tree
  as it appears in the qdocconf file.
 */
Tree::Tree(const QString &camelCaseModuleName, QDocDatabase *qdb)
    : treeHasBeenAnalyzed_(false),
      docsHaveBeenGenerated_(false),
      linkCount_(0),
      camelCaseModuleName_(camelCaseModuleName),
      physicalModuleName_(camelCaseModuleName.toLower()),
      qdb_(qdb),
      root_(nullptr, QString()),
      targetListMap_(nullptr)
{
    root_.setPhysicalModuleName(physicalModuleName_);
    root_.setTree(this);
    const auto &config = Config::instance();
    if (config.getBool(CONFIG_WRITEQAPAGES))
        targetListMap_ = new TargetListMap;
}

/*!
  Destroys the Tree. The root node is a data member
  of this object, so its destructor is called. The
  destructor of each child node is called, and these
  destructors are recursive. Thus the entire tree is
  destroyed.

  There are two maps of targets, keywords, and contents.
  One map is indexed by ref, the other by title. The ref
  is just the canonical form of the title. Both maps
  use the same set of TargetRec objects as the values,
  so the destructor only deletes the values from one of
  the maps. Then it clears both maps.
 */
Tree::~Tree()
{
    for (auto i = nodesByTargetRef_.begin(); i != nodesByTargetRef_.end(); ++i) {
        delete i.value();
    }
    nodesByTargetRef_.clear();
    nodesByTargetTitle_.clear();
    const auto &config = Config::instance();
    if (config.getBool(CONFIG_WRITEQAPAGES) && targetListMap_) {
        for (auto target = targetListMap_->begin(); target != targetListMap_->end(); ++target) {
            TargetList *tlist = target.value();
            if (tlist) {
                for (auto *location : qAsConst(*tlist))
                    delete location;
            }
            delete tlist;
        }
    }
}

/* API members */

/*!
  Calls findClassNode() first with \a path and \a start. If
  it finds a node, the node is returned. If not, it calls
  findNamespaceNode() with the same parameters. The result
  is returned.
 */
Node *Tree::findNodeForInclude(const QStringList &path) const
{
    Node *n = findClassNode(path);
    if (n == nullptr)
        n = findNamespaceNode(path);
    return n;
}

/*!
  This function searches this tree for an Aggregate node with
  the specified \a name. It returns the pointer to that node
  or nullptr.

  We might need to split the name on '::' but we assume the
  name is a single word at the moment.
 */
Aggregate *Tree::findAggregate(const QString &name)
{
    QStringList path = name.split(QLatin1String("::"));
    return static_cast<Aggregate *>(findNodeRecursive(path, 0, const_cast<NamespaceNode *>(root()),
                                                      &Node::isFirstClassAggregate));
}

/*!
  Find the C++ class node named \a path. Begin the search at the
  \a start node. If the \a start node is 0, begin the search
  at the root of the tree. Only a C++ class node named \a path is
  acceptible. If one is not found, 0 is returned.
 */
ClassNode *Tree::findClassNode(const QStringList &path, const Node *start) const
{
    if (start == nullptr)
        start = const_cast<NamespaceNode *>(root());
    return static_cast<ClassNode *>(findNodeRecursive(path, 0, start, &Node::isClassNode));
}

/*!
  Find the Namespace node named \a path. Begin the search at
  the root of the tree. Only a Namespace node named \a path
  is acceptible. If one is not found, 0 is returned.
 */
NamespaceNode *Tree::findNamespaceNode(const QStringList &path) const
{
    Node *start = const_cast<NamespaceNode *>(root());
    return static_cast<NamespaceNode *>(findNodeRecursive(path, 0, start, &Node::isNamespace));
}

/*!
  Find the Qml type node named \a path. Begin the search at the
  \a start node. If the \a start node is 0, begin the search
  at the root of the tree. Only a Qml type node named <\a path is
  acceptible. If one is not found, 0 is returned.
 */
QmlTypeNode *Tree::findQmlTypeNode(const QStringList &path)
{
    /*
      If the path contains one or two double colons ("::"),
      check first to see if the first two path strings refer
      to a QML element. If they do, path[0] will be the QML
      module identifier, and path[1] will be the QML type.
      If the anser is yes, the reference identifies a QML
      class node.
    */
    if (path.size() >= 2 && !path[0].isEmpty()) {
        QmlTypeNode *qcn = qdb_->findQmlType(path[0], path[1]);
        if (qcn != nullptr)
            return qcn;
    }
    return static_cast<QmlTypeNode *>(findNodeRecursive(path, 0, root(), &Node::isQmlType));
}

/*!
  This function searches for the node specified by \a path.
  The matching node can be one of several different types
  including a C++ class, a C++ namespace, or a C++ header
  file.

  I'm not sure if it can be a QML type, but if that is a
  possibility, the code can easily accommodate it.

  If a matching node is found, a pointer to it is returned.
  Otherwise 0 is returned.
 */
Aggregate *Tree::findRelatesNode(const QStringList &path)
{
    Node *n = findNodeRecursive(path, 0, root(), &Node::isRelatableType);
    return (((n != nullptr) && n->isAggregate()) ? static_cast<Aggregate *>(n) : nullptr);
}

/*!
  Inserts function name \a funcName and function role \a funcRole into
  the property function map for the specified \a property.
 */
void Tree::addPropertyFunction(PropertyNode *property, const QString &funcName,
                               PropertyNode::FunctionRole funcRole)
{
    unresolvedPropertyMap[property].insert(funcRole, funcName);
}

/*!
  This function resolves C++ inheritance and reimplementation
  settings for each C++ class node found in the tree beginning
  at \a n. It also calls itself recursively for each C++ class
  node or namespace node it encounters.

  This function does not resolve QML inheritance.
 */
void Tree::resolveBaseClasses(Aggregate *n)
{
    for (auto it = n->constBegin(); it != n->constEnd(); ++it) {
        if ((*it)->isClassNode()) {
            ClassNode *cn = static_cast<ClassNode *>(*it);
            QVector<RelatedClass> &bases = cn->baseClasses();
            for (auto &base : bases) {
                if (base.node_ == nullptr) {
                    Node *n = qdb_->findClassNode(base.path_);
                    /*
                      If the node for the base class was not found,
                      the reason might be that the subclass is in a
                      namespace and the base class is in the same
                      namespace, but the base class name was not
                      qualified with the namespace name. That is the
                      case most of the time. Then restart the search
                      at the parent of the subclass node (the namespace
                      node) using the unqualified base class name.
                    */
                    if (n == nullptr) {
                        Aggregate *parent = cn->parent();
                        if (parent != nullptr)
                            // Exclude the root namespace
                            if (parent->isNamespace() && !parent->name().isEmpty())
                                n = findClassNode(base.path_, parent);
                    }
                    if (n != nullptr) {
                        ClassNode *bcn = static_cast<ClassNode *>(n);
                        base.node_ = bcn;
                        bcn->addDerivedClass(base.access_, cn);
                    }
                }
            }
            resolveBaseClasses(cn);
        } else if ((*it)->isNamespace()) {
            resolveBaseClasses(static_cast<NamespaceNode *>(*it));
        }
    }
}

/*!
 */
void Tree::resolvePropertyOverriddenFromPtrs(Aggregate *n)
{
    for (auto node = n->constBegin(); node != n->constEnd(); ++node) {
        if ((*node)->isClassNode()) {
            ClassNode *cn = static_cast<ClassNode *>(*node);
            for (auto property = cn->constBegin(); property != cn->constEnd(); ++property) {
                if ((*property)->isProperty())
                    cn->resolvePropertyOverriddenFromPtrs(static_cast<PropertyNode *>(*property));
            }
            resolvePropertyOverriddenFromPtrs(cn);
        } else if ((*node)->isNamespace()) {
            resolvePropertyOverriddenFromPtrs(static_cast<NamespaceNode *>(*node));
        }
    }
}

/*!
 */
void Tree::resolveProperties()
{
    for (auto propEntry = unresolvedPropertyMap.constBegin();
         propEntry != unresolvedPropertyMap.constEnd(); ++propEntry) {
        PropertyNode *property = propEntry.key();
        Aggregate *parent = property->parent();
        QString getterName = (*propEntry)[PropertyNode::Getter];
        QString setterName = (*propEntry)[PropertyNode::Setter];
        QString resetterName = (*propEntry)[PropertyNode::Resetter];
        QString notifierName = (*propEntry)[PropertyNode::Notifier];

        for (auto it = parent->constBegin(); it != parent->constEnd(); ++it) {
            if ((*it)->isFunction()) {
                FunctionNode *function = static_cast<FunctionNode *>(*it);
                if (function->access() == property->access()
                    && (function->status() == property->status() || function->doc().isEmpty())) {
                    if (function->name() == getterName) {
                        property->addFunction(function, PropertyNode::Getter);
                    } else if (function->name() == setterName) {
                        property->addFunction(function, PropertyNode::Setter);
                    } else if (function->name() == resetterName) {
                        property->addFunction(function, PropertyNode::Resetter);
                    } else if (function->name() == notifierName) {
                        property->addSignal(function, PropertyNode::Notifier);
                    }
                }
            }
        }
    }

    for (auto propEntry = unresolvedPropertyMap.constBegin();
         propEntry != unresolvedPropertyMap.constEnd(); ++propEntry) {
        PropertyNode *property = propEntry.key();
        // redo it to set the property functions
        if (property->overriddenFrom())
            property->setOverriddenFrom(property->overriddenFrom());
    }

    unresolvedPropertyMap.clear();
}

/*!
  For each QML class node that points to a C++ class node,
  follow its C++ class node pointer and set the C++ class
  node's QML class node pointer back to the QML class node.
 */
void Tree::resolveCppToQmlLinks()
{

    const NodeList &children = root_.childNodes();
    for (auto *child : children) {
        if (child->isQmlType() || child->isJsType()) {
            QmlTypeNode *qcn = static_cast<QmlTypeNode *>(child);
            ClassNode *cn = const_cast<ClassNode *>(qcn->classNode());
            if (cn)
                cn->setQmlElement(qcn);
        }
    }
}

/*!
  For each C++ class node, resolve any \c using clauses
  that appeared in the class declaration.

  For type aliases, resolve the aliased node.
 */
void Tree::resolveUsingClauses(Aggregate *parent)
{
    if (!parent)
        parent = &root_;
    for (auto *child : parent->childNodes()) {
        if (child->isClassNode()) {
            ClassNode *cn = static_cast<ClassNode *>(child);
            QVector<UsingClause> &usingClauses = cn->usingClauses();
            for (auto &usingClause : usingClauses) {
                if (usingClause.node() == nullptr) {
                    const Node *n = qdb_->findFunctionNode(usingClause.signature(), cn, Node::CPP);
                    if (n != nullptr)
                        usingClause.setNode(n);
                }
            }
        } else if (child->isTypeAlias()) {
            TypeAliasNode *ta = static_cast<TypeAliasNode *>(child);
            ta->setAliasedNode(qdb_->findNodeForTarget(ta->aliasedType(), child->parent()));
        }

    if (child->genus() == Node::CPP && child->isAggregate())
        resolveUsingClauses(static_cast<Aggregate *>(child));
    }
}

/*!
  Traverse this Tree and for each ClassNode found, remove
  from its list of base classes any that are marked private
  or internal. When a class is removed from a base class
  list, promote its public pase classes to be base classes
  of the class where the base class was removed. This is
  done for documentation purposes. The function is recursive
  on namespace nodes.
 */
void Tree::removePrivateAndInternalBases(NamespaceNode *rootNode)
{
    if (rootNode == nullptr)
        rootNode = root();

    for (auto node = rootNode->constBegin(); node != rootNode->constEnd(); ++node) {
        if ((*node)->isClassNode())
            static_cast<ClassNode *>(*node)->removePrivateAndInternalBases();
        else if ((*node)->isNamespace())
            removePrivateAndInternalBases(static_cast<NamespaceNode *>(*node));
    }
}

/*!
 */
ClassList Tree::allBaseClasses(const ClassNode *classNode) const
{
    ClassList result;
    const auto &baseClasses = classNode->baseClasses();
    for (const auto &relatedClass : baseClasses) {
        if (relatedClass.node_ != nullptr) {
            result += relatedClass.node_;
            result += allBaseClasses(relatedClass.node_);
        }
    }
    return result;
}

/*!
  Find the node with the specified \a path name that is of
  the specified \a type and \a subtype. Begin the search at
  the \a start node. If the \a start node is 0, begin the
  search at the tree root. \a subtype is not used unless
  \a type is \c{Page}.
 */
Node *Tree::findNodeByNameAndType(const QStringList &path, bool (Node::*isMatch)() const) const
{
    return findNodeRecursive(path, 0, root(), isMatch);
}

/*!
  Recursive search for a node identified by \a path. Each
  path element is a name. \a pathIndex specifies the index
  of the name in \a path to try to match. \a start is the
  node whose children shoulod be searched for one that has
  that name. Each time a match is found, increment the
  \a pathIndex and call this function recursively.

  If the end of the path is reached (i.e. if a matching
  node is found for each name in the \a path), the \a type
  must match the type of the last matching node, and if the
  type is \e{Page}, the \a subtype must match as well.

  If the algorithm is successful, the pointer to the final
  node is returned. Otherwise 0 is returned.
 */
Node *Tree::findNodeRecursive(const QStringList &path, int pathIndex, const Node *start,
                              bool (Node::*isMatch)() const) const
{
    if (start == nullptr || path.isEmpty())
        return nullptr;
    Node *node = const_cast<Node *>(start);
    if (!node->isAggregate())
        return ((pathIndex >= path.size()) ? node : nullptr);
    Aggregate *current = static_cast<Aggregate *>(node);
    const NodeList &children = current->childNodes();
    const QString &name = path.at(pathIndex);
    for (auto *node : children) {
        if (node == nullptr)
            continue;
        if (node->name() == name) {
            if (pathIndex + 1 >= path.size()) {
                if ((node->*(isMatch))())
                    return node;
                continue;
            } else { // Search the children of n for the next name in the path.
                node = findNodeRecursive(path, pathIndex + 1, node, isMatch);
                if (node != nullptr)
                    return node;
            }
        }
    }
    return nullptr;
}

/*!
  Searches the tree for a node that matches the \a path plus
  the \a target. The search begins at \a start and moves up
  the parent chain from there, or, if \a start is 0, the search
  begins at the root.

  The \a flags can indicate whether to search base classes and/or
  the enum values in enum types. \a genus can be a further restriction
  on what kind of node is an acceptible match, i.e. CPP or QML.

  If a matching node is found, \a ref is an output parameter that
  is set to the HTML reference to use for the link.
 */
const Node *Tree::findNodeForTarget(const QStringList &path, const QString &target,
                                    const Node *start, int flags, Node::Genus genus,
                                    QString &ref) const
{
    const Node *node = nullptr;
    if ((genus == Node::DontCare) || (genus == Node::DOC)) {
        node = findPageNodeByTitle(path.at(0));
        if (node) {
            if (!target.isEmpty()) {
                ref = getRef(target, node);
                if (ref.isEmpty())
                    node = nullptr;
            }
            if (node)
                return node;
        }
    }

    node = findUnambiguousTarget(path.join(QLatin1String("::")), genus, ref);
    if (node) {
        if (!target.isEmpty()) {
            ref = getRef(target, node);
            if (ref.isEmpty())
                node = nullptr;
        }
        if (node)
            return node;
    }

    const Node *current = start;
    if (current == nullptr)
        current = root();

    /*
      If the path contains one or two double colons ("::"),
      check first to see if the first two path strings refer
      to a QML element. If they do, path[0] will be the QML
      module identifier, and path[1] will be the QML type.
      If the answer is yes, the reference identifies a QML
      type node.
    */
    int path_idx = 0;
    if (((genus == Node::QML) || (genus == Node::DontCare)) && (path.size() >= 2)
        && !path[0].isEmpty()) {
        QmlTypeNode *qcn = lookupQmlType(QString(path[0] + "::" + path[1]));
        if (qcn) {
            current = qcn;
            if (path.size() == 2) {
                if (!target.isEmpty()) {
                    ref = getRef(target, current);
                    if (!ref.isEmpty())
                        return current;
                    return nullptr;
                } else
                    return current;
            }
            path_idx = 2;
        }
    }

    while (current != nullptr) {
        if (current->isAggregate()) { // Should this be isPageNode() ???
            const Node *node =
                    matchPathAndTarget(path, path_idx, target, current, flags, genus, ref);
            if (node)
                return node;
        }
        current = current->parent();
        path_idx = 0;
    }
    return nullptr;
}

/*!
  First, the \a path is used to find a node. The \a path
  matches some part of the node's fully quallified name.
  If the \a target is not empty, it must match a target
  in the matching node. If the matching of the \a path
  and the \a target (if present) is successful, \a ref
  is set from the \a target, and the pointer to the
  matching node is returned. \a idx is the index into the
  \a path where to begin the matching. The function is
  recursive with idx being incremented for each recursive
  call.

  The matching node must be of the correct \a genus, i.e.
  either QML or C++, but \a genus can be set to \c DontCare.
  \a flags indicates whether to search base classes and
  whether to search for an enum value. \a node points to
  the node where the search should begin, assuming the
  \a path is a not a fully-qualified name. \a node is
  most often the root of this Tree.
 */
const Node *Tree::matchPathAndTarget(const QStringList &path, int idx, const QString &target,
                                     const Node *node, int flags, Node::Genus genus,
                                     QString &ref) const
{
    /*
      If the path has been matched, then if there is a target,
      try to match the target. If there is a target, but you
      can't match it at the end of the path, give up; return 0.
     */
    if (idx == path.size()) {
        if (!target.isEmpty()) {
            ref = getRef(target, node);
            if (ref.isEmpty())
                return nullptr;
        }
        if (node->isFunction() && node->name() == node->parent()->name())
            node = node->parent();
        return node;
    }

    QString name = path.at(idx);
    if (node->isAggregate()) {
        NodeVector nodes;
        static_cast<const Aggregate *>(node)->findChildren(name, nodes);
        for (const auto *node : qAsConst(nodes)) {
            if (genus != Node::DontCare && node->genus() != genus)
                continue;
            const Node *t = matchPathAndTarget(path, idx + 1, target, node, flags, genus, ref);
            if (t && !t->isPrivate())
                return t;
        }
    }
    if (target.isEmpty()) {
        if ((idx) == (path.size() - 1) && node->isAggregate() && (flags & SearchEnumValues)) {
            const Node *t =
                    static_cast<const Aggregate *>(node)->findEnumNodeForValue(path.at(idx));
            if (t)
                return t;
        }
    }
    if (((genus == Node::CPP) || (genus == Node::DontCare)) && node->isClassNode()
        && (flags & SearchBaseClasses)) {
        const ClassList bases = allBaseClasses(static_cast<const ClassNode *>(node));
        for (const auto *base : bases) {
            const Node *t = matchPathAndTarget(path, idx, target, base, flags, genus, ref);
            if (t && !t->isPrivate())
                return t;
            if (target.isEmpty()) {
                if ((idx) == (path.size() - 1) && (flags & SearchEnumValues)) {
                    t = base->findEnumNodeForValue(path.at(idx));
                    if (t)
                        return t;
                }
            }
        }
    }
    return nullptr;
}

/*!
  Searches the tree for a node that matches the \a path. The
  search begins at \a start but can move up the parent chain
  recursively if no match is found. The \a flags are used to
  restrict the search.
 */
const Node *Tree::findNode(const QStringList &path, const Node *start, int flags,
                           Node::Genus genus) const
{
    const Node *current = start;
    if (current == nullptr)
        current = root();

    do {
        const Node *node = current;
        int i;
        int start_idx = 0;

        /*
          If the path contains one or two double colons ("::"),
          check first to see if the first two path strings refer
          to a QML element. If they do, path[0] will be the QML
          module identifier, and path[1] will be the QML type.
          If the answer is yes, the reference identifies a QML
          type node.
        */
        if (((genus == Node::QML) || (genus == Node::DontCare)) && (path.size() >= 2)
            && !path[0].isEmpty()) {
            QmlTypeNode *qcn = lookupQmlType(QString(path[0] + "::" + path[1]));
            if (qcn != nullptr) {
                node = qcn;
                if (path.size() == 2)
                    return node;
                start_idx = 2;
            }
        }

        for (i = start_idx; i < path.size(); ++i) {
            if (node == nullptr || !node->isAggregate())
                break;

            // Clear the TypesOnly flag until the last path segment, as e.g. namespaces are not
            // types. We also ignore module nodes as they are not aggregates and thus have no
            // children.
            int tmpFlags = (i < path.size() - 1) ? (flags & ~TypesOnly) | IgnoreModules : flags;

            const Node *next = static_cast<const Aggregate *>(node)->findChildNode(path.at(i),
                                                                                   genus, tmpFlags);
            if ((next == nullptr) && (flags & SearchEnumValues) && i == path.size() - 1) {
                next = static_cast<const Aggregate *>(node)->findEnumNodeForValue(path.at(i));
            }
            if ((next == nullptr) && ((genus == Node::CPP) || (genus == Node::DontCare))
                && node->isClassNode() && (flags & SearchBaseClasses)) {
                const ClassList bases = allBaseClasses(static_cast<const ClassNode *>(node));
                for (const auto *base : bases) {
                    next = base->findChildNode(path.at(i), genus, tmpFlags);
                    if ((next == nullptr) && (flags & SearchEnumValues) && i == path.size() - 1)
                        next = base->findEnumNodeForValue(path.at(i));
                    if (next != nullptr)
                        break;
                }
            }
            node = next;
        }
        if ((node != nullptr) && i == path.size())
            return node;
        current = current->parent();
    } while (current != nullptr);

    return nullptr;
}

/*!
  This function searches for a node with a canonical title
  constructed from \a target. If the node it finds is \a node,
  it returns the ref from that node. Otherwise it returns an
  empty string.
 */
QString Tree::getRef(const QString &target, const Node *node) const
{
    auto it = nodesByTargetTitle_.constFind(target);
    if (it != nodesByTargetTitle_.constEnd()) {
        do {
            if (it.value()->node_ == node)
                return it.value()->ref_;
            ++it;
        } while (it != nodesByTargetTitle_.constEnd() && it.key() == target);
    }
    QString key = Doc::canonicalTitle(target);
    it = nodesByTargetRef_.constFind(key);
    if (it != nodesByTargetRef_.constEnd()) {
        do {
            if (it.value()->node_ == node)
                return it.value()->ref_;
            ++it;
        } while (it != nodesByTargetRef_.constEnd() && it.key() == key);
    }
    return QString();
}

/*!
  Inserts a new target into the target table. \a name is the
  key. The target record contains the \a type, a pointer to
  the \a node, the \a priority. and a canonicalized form of
  the \a name, which is later used.
 */
void Tree::insertTarget(const QString &name, const QString &title, TargetRec::TargetType type,
                        Node *node, int priority)
{
    TargetRec *target = new TargetRec(name, title, type, node, priority);
    nodesByTargetRef_.insert(name, target);
    nodesByTargetTitle_.insert(title, target);
}

/*!
 */
void Tree::resolveTargets(Aggregate *root)
{
    for (auto *child : root->childNodes()) {
        if (child->isTextPageNode()) {
            PageNode *node = static_cast<PageNode *>(child);
            QString key = node->title();
            if (!key.isEmpty()) {
                if (key.contains(QChar(' ')))
                    key = Doc::canonicalTitle(key);
                QList<PageNode *> nodes = pageNodesByTitle_.values(key);
                bool alreadyThere = false;
                if (!nodes.empty()) {
                    for (const auto &node_ : nodes) {
                        if (node_->isExternalPage()) {
                            if (node->name() == node_->name()) {
                                alreadyThere = true;
                                break;
                            }
                        }
                    }
                }
                if (!alreadyThere)
                    pageNodesByTitle_.insert(key, node);
            }
        }

        if (child->doc().hasTableOfContents()) {
            const QVector<Atom *> &toc = child->doc().tableOfContents();
            for (int i = 0; i < toc.size(); ++i) {
                QString ref = refForAtom(toc.at(i));
                QString title = Text::sectionHeading(toc.at(i)).toString();
                if (!ref.isEmpty() && !title.isEmpty()) {
                    QString key = Doc::canonicalTitle(title);
                    TargetRec *target = new TargetRec(ref, title, TargetRec::Contents, child, 3);
                    nodesByTargetRef_.insert(key, target);
                    nodesByTargetTitle_.insert(title, target);
                }
            }
        }
        if (child->doc().hasKeywords()) {
            const QVector<Atom *> &keywords = child->doc().keywords();
            for (int i = 0; i < keywords.size(); ++i) {
                QString ref = refForAtom(keywords.at(i));
                QString title = keywords.at(i)->string();
                if (!ref.isEmpty() && !title.isEmpty()) {
                    TargetRec *target = new TargetRec(ref, title, TargetRec::Keyword, child, 1);
                    nodesByTargetRef_.insert(Doc::canonicalTitle(title), target);
                    nodesByTargetTitle_.insert(title, target);
                }
            }
        }
        if (child->doc().hasTargets()) {
            const QVector<Atom *> &targets = child->doc().targets();
            for (int i = 0; i < targets.size(); ++i) {
                QString ref = refForAtom(targets.at(i));
                QString title = targets.at(i)->string();
                if (!ref.isEmpty() && !title.isEmpty()) {
                    QString key = Doc::canonicalTitle(title);
                    TargetRec *target = new TargetRec(ref, title, TargetRec::Target, child, 2);
                    nodesByTargetRef_.insert(key, target);
                    nodesByTargetTitle_.insert(title, target);
                }
            }
        }
        if (child->isAggregate())
            resolveTargets(static_cast<Aggregate *>(child));
    }
}

/*!
  This function searches for a \a target anchor node. If it
  finds one, it sets \a ref and returns the found node.
 */
const Node *Tree::findUnambiguousTarget(const QString &target, Node::Genus genus,
                                        QString &ref) const
{
    int numBestTargets = 0;
    TargetRec *bestTarget = nullptr;
    QVector<TargetRec *> bestTargetList;

    QString key = target;
    for (auto it = nodesByTargetTitle_.find(key); it != nodesByTargetTitle_.constEnd(); ++it) {
        if (it.key() != key)
            break;
        TargetRec *candidate = it.value();
        if ((genus == Node::DontCare) || (genus == candidate->genus())) {
            if (!bestTarget || (candidate->priority_ < bestTarget->priority_)) {
                bestTarget = candidate;
                bestTargetList.clear();
                bestTargetList.append(candidate);
                numBestTargets = 1;
            } else if (candidate->priority_ == bestTarget->priority_) {
                bestTargetList.append(candidate);
                ++numBestTargets;
            }
        }
    }
    if (bestTarget) {
        ref = bestTarget->ref_;
        return bestTarget->node_;
    }

    numBestTargets = 0;
    bestTarget = nullptr;
    key = Doc::canonicalTitle(target);
    for (auto it = nodesByTargetRef_.find(key); it != nodesByTargetRef_.constEnd(); ++it) {
        if (it.key() != key)
            break;
        TargetRec *candidate = it.value();
        if ((genus == Node::DontCare) || (genus == candidate->genus())) {
            if (!bestTarget || (candidate->priority_ < bestTarget->priority_)) {
                bestTarget = candidate;
                bestTargetList.clear();
                bestTargetList.append(candidate);
                numBestTargets = 1;
            } else if (candidate->priority_ == bestTarget->priority_) {
                bestTargetList.append(candidate);
                ++numBestTargets;
            }
        }
    }
    if (bestTarget) {
        ref = bestTarget->ref_;
        return bestTarget->node_;
    }

    ref.clear();
    return nullptr;
}

/*!
  This function searches for a node with the specified \a title.
 */
const PageNode *Tree::findPageNodeByTitle(const QString &title) const
{
    PageNodeMultiMap::const_iterator it;
    if (title.contains(QChar(' ')))
        it = pageNodesByTitle_.constFind(Doc::canonicalTitle(title));
    else
        it = pageNodesByTitle_.constFind(title);
    if (it != pageNodesByTitle_.constEnd()) {
        /*
          Reporting all these duplicate section titles is probably
          overkill. We should report the duplicate file and let
          that suffice.
        */
        PageNodeMultiMap::const_iterator j = it;
        ++j;
        if (j != pageNodesByTitle_.constEnd() && j.key() == it.key()) {
            while (j != pageNodesByTitle_.constEnd()) {
                if (j.key() == it.key() && j.value()->url().isEmpty()) {
                    break; // Just report one duplicate for now.
                }
                ++j;
            }
            if (j != pageNodesByTitle_.cend()) {
                it.value()->location().warning("This page title exists in more than one file: "
                                               + title);
                j.value()->location().warning("[It also exists here]");
            }
        }
        return it.value();
    }
    return nullptr;
}

/*!
  Returns a canonical title for the \a atom, if the \a atom
  is a SectionLeft or a Target.
 */
QString Tree::refForAtom(const Atom *atom)
{
    if (atom) {
        if (atom->type() == Atom::SectionLeft)
            return Doc::canonicalTitle(Text::sectionHeading(atom).toString());
        if ((atom->type() == Atom::Target) || (atom->type() == Atom::Keyword))
            return Doc::canonicalTitle(atom->string());
    }
    return QString();
}

/*!
  \fn const CNMap &Tree::groups() const
  Returns a const reference to the collection of all
  group nodes.
*/

/*!
  \fn const ModuleMap &Tree::modules() const
  Returns a const reference to the collection of all
  module nodes.
*/

/*!
  \fn const QmlModuleMap &Tree::qmlModules() const
  Returns a const reference to the collection of all
  QML module nodes.
*/

/*!
  Returns a pointer to the collection map specified by \a type.
  Returns null if \a type is not specified.
 */
CNMap *Tree::getCollectionMap(Node::NodeType type)
{
    switch (type) {
    case Node::Group:
        return &groups_;
    case Node::Module:
        return &modules_;
    case Node::QmlModule:
        return &qmlModules_;
    case Node::JsModule:
        return &jsModules_;
    default:
        break;
    }
    return nullptr;
}

/*!
  Searches this tree for a collection named \a name with the
  specified \a type. If the collection is found, a pointer
  to it is returned. If a collection is not found, null is
  returned.
 */
CollectionNode *Tree::getCollection(const QString &name, Node::NodeType type)
{
    CNMap *map = getCollectionMap(type);
    if (map) {
        auto it = map->constFind(name);
        if (it != map->cend())
            return it.value();
    }
    return nullptr;
}

/*!
  Find the group, module, QML module, or JavaScript module
  named \a name and return a pointer to that collection node.
  \a type specifies which kind of collection node you want.
  If a collection node with the specified \a name and \a type
  is not found, a new one is created, and the pointer to the
  new one is returned.

  If a new collection node is created, its parent is the tree
  root, and the new collection node is marked \e{not seen}.

  \a genus must be specified, i.e. it must not be \c{DontCare}.
  If it is \c{DontCare}, 0 is returned, which is a programming
  error.
 */
CollectionNode *Tree::findCollection(const QString &name, Node::NodeType type)
{
    CNMap *m = getCollectionMap(type);
    if (!m) // error
        return nullptr;
    auto it = m->constFind(name);
    if (it != m->cend())
        return it.value();
    CollectionNode *cn = new CollectionNode(type, root(), name);
    cn->markNotSeen();
    m->insert(name, cn);
    return cn;
}

/*! \fn CollectionNode *Tree::findGroup(const QString &name)
  Find the group node named \a name and return a pointer
  to it. If the group node is not found, add a new group
  node named \a name and return a pointer to the new one.

  If a new group node is added, its parent is the tree root,
  and the new group node is marked \e{not seen}.
 */

/*! \fn CollectionNode *Tree::findModule(const QString &name)
  Find the module node named \a name and return a pointer
  to it. If a matching node is not found, add a new module
  node named \a name and return a pointer to that one.

  If a new module node is added, its parent is the tree root,
  and the new module node is marked \e{not seen}.
 */

/*! \fn CollectionNode *Tree::findQmlModule(const QString &name)
  Find the QML module node named \a name and return a pointer
  to it. If a matching node is not found, add a new QML module
  node named \a name and return a pointer to that one.

  If a new QML module node is added, its parent is the tree root,
  and the new node is marked \e{not seen}.
 */

/*! \fn CollectionNode *Tree::findJsModule(const QString &name)
  Find the JavaScript module named \a name and return a pointer
  to it. If a matching node is not found, add a new JavaScript
  module node named \a name and return a pointer to that one.

  If a new JavaScript module node is added, its parent is the
  tree root, and the new node is marked \e{not seen}.
 */

/*! \fn CollectionNode *Tree::addGroup(const QString &name)
  Looks up the group node named \a name in the collection
  of all group nodes. If a match is found, a pointer to the
  node is returned. Otherwise, a new group node named \a name
  is created and inserted into the collection, and the pointer
  to that node is returned.
 */

/*! \fn CollectionNode *Tree::addModule(const QString &name)
  Looks up the module node named \a name in the collection
  of all module nodes. If a match is found, a pointer to the
  node is returned. Otherwise, a new module node named \a name
  is created and inserted into the collection, and the pointer
  to that node is returned.
 */

/*! \fn CollectionNode *Tree::addQmlModule(const QString &name)
  Looks up the QML module node named \a name in the collection
  of all QML module nodes. If a match is found, a pointer to the
  node is returned. Otherwise, a new QML module node named \a name
  is created and inserted into the collection, and the pointer
  to that node is returned.
 */

/*! \fn CollectionNode *Tree::addJsModule(const QString &name)
  Looks up the JavaScript module node named \a name in the collection
  of all JavaScript module nodes. If a match is found, a pointer to the
  node is returned. Otherwise, a new JavaScrpt module node named \a name
  is created and inserted into the collection, and the pointer
  to that node is returned.
 */

/*!
  Looks up the group node named \a name in the collection
  of all group nodes. If a match is not found, a new group
  node named \a name is created and inserted into the collection.
  Then append \a node to the group's members list, and append the
  group name to the list of group names in \a node. The parent of
  \a node is not changed by this function. Returns a pointer to
  the group node.
 */
CollectionNode *Tree::addToGroup(const QString &name, Node *node)
{
    CollectionNode *cn = findGroup(name);
    if (!node->isInternal()) {
        cn->addMember(node);
        node->appendGroupName(name);
    }
    return cn;
}

/*!
  Looks up the module node named \a name in the collection
  of all module nodes. If a match is not found, a new module
  node named \a name is created and inserted into the collection.
  Then append \a node to the module's members list. The parent of
  \a node is not changed by this function. Returns the module node.
 */
CollectionNode *Tree::addToModule(const QString &name, Node *node)
{
    CollectionNode *cn = findModule(name);
    cn->addMember(node);
    node->setPhysicalModuleName(name);
    return cn;
}

/*!
  Looks up the QML module named \a name. If it isn't there,
  create it. Then append \a node to the QML module's member
  list. The parent of \a node is not changed by this function.
  Returns the pointer to the QML module node.
 */
CollectionNode *Tree::addToQmlModule(const QString &name, Node *node)
{
    QStringList qmid;
    QStringList dotSplit;
    QStringList blankSplit = name.split(QLatin1Char(' '));
    qmid.append(blankSplit[0]);
    if (blankSplit.size() > 1) {
        qmid.append(blankSplit[0] + blankSplit[1]);
        dotSplit = blankSplit[1].split(QLatin1Char('.'));
        qmid.append(blankSplit[0] + dotSplit[0]);
    }

    CollectionNode *cn = findQmlModule(blankSplit[0]);
    cn->addMember(node);
    node->setQmlModule(cn);
    if (node->isQmlType()) {
        QmlTypeNode *n = static_cast<QmlTypeNode *>(node);
        for (int i = 0; i < qmid.size(); ++i) {
            QString key = qmid[i] + "::" + node->name();
            insertQmlType(key, n);
        }
    }
    return cn;
}

/*!
  Looks up the QML module named \a name. If it isn't there,
  create it. Then append \a node to the QML module's member
  list. The parent of \a node is not changed by this function.
  Returns the pointer to the QML module node.
 */
CollectionNode *Tree::addToJsModule(const QString &name, Node *node)
{
    QStringList qmid;
    QStringList dotSplit;
    QStringList blankSplit = name.split(QLatin1Char(' '));
    qmid.append(blankSplit[0]);
    if (blankSplit.size() > 1) {
        qmid.append(blankSplit[0] + blankSplit[1]);
        dotSplit = blankSplit[1].split(QLatin1Char('.'));
        qmid.append(blankSplit[0] + dotSplit[0]);
    }

    CollectionNode *cn = findJsModule(blankSplit[0]);
    cn->addMember(node);
    node->setQmlModule(cn);
    if (node->isJsType()) {
        QmlTypeNode *n = static_cast<QmlTypeNode *>(node);
        for (int i = 0; i < qmid.size(); ++i) {
            QString key = qmid[i] + "::" + node->name();
            insertQmlType(key, n);
        }
    }
    return cn;
}

/*!
  If the QML type map does not contain \a key, insert node
  \a n with the specified \a key.
 */
void Tree::insertQmlType(const QString &key, QmlTypeNode *n)
{
    if (!qmlTypeMap_.contains(key))
        qmlTypeMap_.insert(key, n);
}

/*!
  Finds the function node with the specifried name \a path that
  also has the specified \a parameters and returns a pointer to
  the the first matching function node if one is found.

  This function begins searching the tree at \a relative for
  the \l {FunctionNode} {function node} identified by \a path
  that has the specified \a parameters. The \a flags are
  used to restrict the search. If a matching node is found, a
  pointer to it is returned. Otherwise, nullis returned. If
  \a relative is ull, the search begins at the tree root.
 */
const FunctionNode *Tree::findFunctionNode(const QStringList &path, const Parameters &parameters,
                                           const Node *relative, Node::Genus genus) const
{
    if (path.size() == 3 && !path[0].isEmpty()
        && ((genus == Node::QML) || (genus == Node::DontCare))) {
        QmlTypeNode *qcn = lookupQmlType(QString(path[0] + "::" + path[1]));
        if (qcn == nullptr) {
            QStringList p(path[1]);
            Node *n = findNodeByNameAndType(p, &Node::isQmlType);
            if ((n != nullptr) && (n->isQmlType() || n->isJsType()))
                qcn = static_cast<QmlTypeNode *>(n);
        }
        if (qcn != nullptr)
            return static_cast<const FunctionNode *>(qcn->findFunctionChild(path[2], parameters));
    }

    if (relative == nullptr)
        relative = root();
    else if (genus != Node::DontCare) {
        if (genus != relative->genus())
            relative = root();
    }

    do {
        Node *node = const_cast<Node *>(relative);
        int i;

        for (i = 0; i < path.size(); ++i) {
            if (node == nullptr || !node->isAggregate())
                break;

            Aggregate *aggregate = static_cast<Aggregate *>(node);
            Node *next = nullptr;
            if (i == path.size() - 1)
                next = aggregate->findFunctionChild(path.at(i), parameters);
            else
                next = aggregate->findChildNode(path.at(i), genus);

            if ((next == nullptr) && aggregate->isClassNode()) {
                const ClassList bases = allBaseClasses(static_cast<const ClassNode *>(aggregate));
                for (auto *base : bases) {
                    if (i == path.size() - 1)
                        next = base->findFunctionChild(path.at(i), parameters);
                    else
                        next = base->findChildNode(path.at(i), genus);

                    if (next != nullptr)
                        break;
                }
            }

            node = next;
        } // for (i = 0; i < path.size(); ++i)

        if (node && i == path.size() && node->isFunction()) {
            // A function node was found at the end of the path.
            // If it is not marked private, return it. If it is
            // marked private, then if it overrides a function,
            // find that function instead because it might not
            // be marked private. If all the overloads are
            // marked private, return the original function node.
            // This should be replace with findOverriddenFunctionNode().
            const FunctionNode *fn = static_cast<const FunctionNode *>(node);
            const FunctionNode *FN = fn;
            while (FN->isPrivate() && !FN->overridesThis().isEmpty()) {
                QStringList path = FN->overridesThis().split("::");
                FN = qdb_->findFunctionNode(path, parameters, relative, genus);
                if (FN == nullptr)
                    break;
                if (!FN->isPrivate())
                    return FN;
            }
            return fn;
        }
        relative = relative->parent();
    } while (relative);
    return nullptr;
}

/*!
  Generate a target of the form link-nnn, where the nnn is
  the current link count for this tree. This target string
  is returned. It will be output as an HTML anchor just before
  an HTML link to the node \a t.

  The node \a t
 */
QString Tree::getNewLinkTarget(const Node *locNode, const Node *t, const QString &fileName,
                               QString &text, bool broken)
{
    QString physicalModuleName;
    if (t != nullptr && !broken) {
        Tree *tree = t->tree();
        if (tree != this)
            tree->incrementLinkCount();
        physicalModuleName = tree->physicalModuleName();
    } else
        physicalModuleName = "broken";
    incrementLinkCount();
    QString target = QString("qa-target-%1").arg(-(linkCount()));
    TargetLoc *tloc = new TargetLoc(locNode, target, fileName, text, broken);
    TargetList *tList = nullptr;
    auto it = targetListMap_->find(physicalModuleName);
    if (it == targetListMap_->end()) {
        tList = new TargetList;
        it = targetListMap_->insert(physicalModuleName, tList);
    } else
        tList = it.value();
    tList->append(tloc);
    return target;
}

/*!
  Look up the target list for the specified \a module
  and return a pointer to it.
 */
TargetList *Tree::getTargetList(const QString &module)
{
    return targetListMap_->value(module);
}

/*!
  Search this tree recursively from \a parent to find a function
  node with the specified \a tag. If no function node is found
  with the required \a tag, return 0.
 */
FunctionNode *Tree::findFunctionNodeForTag(const QString &tag, Aggregate *parent)
{
    if (parent == nullptr)
        parent = root();
    const NodeList &children = parent->childNodes();
    for (Node *n : children) {
        if (n != nullptr && n->isFunction() && n->hasTag(tag))
            return static_cast<FunctionNode *>(n);
    }
    for (Node *n : children) {
        if (n != nullptr && n->isAggregate()) {
            n = findFunctionNodeForTag(tag, static_cast<Aggregate *>(n));
            if (n != nullptr)
                return static_cast<FunctionNode *>(n);
        }
    }
    return nullptr;
}

/*!
  There should only be one macro node for macro name \a t.
  The macro node is not built until the \macro command is seen.
 */
FunctionNode *Tree::findMacroNode(const QString &t, const Aggregate *parent)
{
    if (parent == nullptr)
        parent = root();
    const NodeList &children = parent->childNodes();
    for (Node *n : children) {
        if (n != nullptr && (n->isMacro() || n->isFunction()) && n->name() == t)
            return static_cast<FunctionNode *>(n);
    }
    for (Node *n : children) {
        if (n != nullptr && n->isAggregate()) {
            FunctionNode *fn = findMacroNode(t, static_cast<Aggregate *>(n));
            if (fn != nullptr)
                return fn;
        }
    }
    return nullptr;
}

/*!
  Add the class and struct names in \a arg to the \e {don't document}
  map.
 */
void Tree::addToDontDocumentMap(QString &arg)
{
    arg.remove(QChar('('));
    arg.remove(QChar(')'));
    QString t = arg.simplified();
    QStringList sl = t.split(QChar(' '));
    if (sl.isEmpty())
        return;
    for (const QString &s : sl) {
        if (!dontDocumentMap_.contains(s))
            dontDocumentMap_.insert(s, nullptr);
    }
}

/*!
  The \e {don't document} map has been loaded with the names
  of classes and structs in the current module that are not
  documented and should not be documented. Now traverse the
  map, and for each class or struct name, find the class node
  that represents that class or struct and mark it with the
  \C DontDocument status.

  This results in a map of the class and struct nodes in the
  module that are in the public API but are not meant to be
  used by anyone. They are only used internally, but for one
  reason or another, they must have public visibility.
 */
void Tree::markDontDocumentNodes()
{
    for (auto it = dontDocumentMap_.begin(); it != dontDocumentMap_.end(); ++it) {
        Aggregate *node = findAggregate(it.key());
        if (node != nullptr)
            node->setStatus(Node::DontDocument);
    }
}

QT_END_NAMESPACE
