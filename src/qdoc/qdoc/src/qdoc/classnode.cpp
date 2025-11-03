// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "classnode.h"

#include "functionnode.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "utilities.h"

#include <QtCore/qlatin1stringview.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
  \class ClassNode
  \brief The ClassNode represents a C++ class.

  It is also used to represent a C++ struct or union. There are some
  actual uses for structs, but I don't think any unions have been
  documented yet.
 */

/*!
  Adds the base class \a node to this class's list of base
  classes. The base class has the specified \a access. This
  is a resolved base class.
 */
void ClassNode::addResolvedBaseClass(Access access, ClassNode *node)
{
    m_bases.append(RelatedClass(access, node));
    node->m_derived.append(RelatedClass(access, this));
}

/*!
  Adds the derived class \a node to this class's list of derived
  classes. The derived class inherits this class with \a access.
 */
void ClassNode::addDerivedClass(Access access, ClassNode *node)
{
    m_derived.append(RelatedClass(access, node));
}

/*!
  Add an unresolved base class to this class node's list of
  base classes. The unresolved base class will be resolved
  before the generate phase of qdoc. In an unresolved base
  class, the pointer to the base class node is 0.
 */
void ClassNode::addUnresolvedBaseClass(Access access, const QStringList &path)
{
    m_bases.append(RelatedClass(access, path));
}

/*!
  Search the child list to find the property node with the
  specified \a name.
 */
PropertyNode *ClassNode::findPropertyNode(const QString &name)
{
    Node *n = findNonfunctionChild(name, &Node::isProperty);

    if (n)
        return static_cast<PropertyNode *>(n);

    PropertyNode *pn = nullptr;

    const QList<RelatedClass> &bases = baseClasses();
    if (!bases.isEmpty()) {
        for (const RelatedClass &base : bases) {
            ClassNode *cn = base.m_node;
            if (cn) {
                pn = cn->findPropertyNode(name);
                if (pn)
                    break;
            }
        }
    }
    const QList<RelatedClass> &ignoredBases = ignoredBaseClasses();
    if (!ignoredBases.isEmpty()) {
        for (const RelatedClass &base : ignoredBases) {
            ClassNode *cn = base.m_node;
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
  \a fn is an overriding function in this class or in a class
  derived from this class. Find the node for the function that
  \a fn overrides in this class's children or in one of this
  class's base classes. Return a pointer to the overridden
  function or return 0.

  This should be revised because clang provides the path to the
  overridden function. mws 15/12/2018
 */
FunctionNode *ClassNode::findOverriddenFunction(const FunctionNode *fn)
{
    for (auto &bc : m_bases) {
        ClassNode *cn = bc.m_node;
        if (cn == nullptr) {
            cn = QDocDatabase::qdocDB()->findClassNode(bc.m_path);
            bc.m_node = cn;
        }
        if (cn != nullptr) {
            FunctionNode *result = cn->findFunctionChild(fn);
            if (result != nullptr && !result->isInternal() && !result->isNonvirtual()
                && result->hasDoc())
                return result;
            result = cn->findOverriddenFunction(fn);
            if (result != nullptr && !result->isNonvirtual())
                return result;
        }
    }
    return nullptr;
}

/*!
  \a fn is an overriding function in this class or in a class
  derived from this class. Find the node for the property that
  \a fn overrides in this class's children or in one of this
  class's base classes. Return a pointer to the overridden
  property or return 0.
 */
PropertyNode *ClassNode::findOverriddenProperty(const FunctionNode *fn)
{
    for (auto &baseClass : m_bases) {
        ClassNode *cn = baseClass.m_node;
        if (cn == nullptr) {
            cn = QDocDatabase::qdocDB()->findClassNode(baseClass.m_path);
            baseClass.m_node = cn;
        }
        if (cn != nullptr) {
            const NodeList &children = cn->childNodes();
            for (const auto &child : children) {
                if (child->isProperty()) {
                    auto *pn = static_cast<PropertyNode *>(child);
                    if (pn->name() == fn->name() || pn->hasAccessFunction(fn->name())) {
                        if (pn->hasDoc())
                            return pn;
                    }
                }
            }
            PropertyNode *result = cn->findOverriddenProperty(fn);
            if (result != nullptr)
                return result;
        }
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
  \brief Detects circular relationships in class hierarchies.

  Traverses the hierarchy in the specified \a direction (Base or Derived),
  checking only private/internal nodes to detect cycles that could cause
  infinite loops during base/derived class promotion.

  If \a cyclePath is provided, it will be filled with the names of
  classes forming the cycle (e.g., ["A", "B", "C", "A"]).

  Returns true if a cycle is detected.

  \sa detectCycleRecursive().
 */
bool ClassNode::hasCircularRelationship(HierarchyDirection direction,
                                        QStringList *cyclePath) const
{
    QMap<const ClassNode*, Color> colors;
    QList<const ClassNode*> path;

    bool cycleFound = detectCycleRecursive(direction, colors, path);

    if (cycleFound && cyclePath) {
        cyclePath->clear();

        if (!path.isEmpty()) {
            const ClassNode *repeatedNode = path.last();
            auto first = path.indexOf(repeatedNode);
            if (first != -1 && first < path.size() - 1)
                path = path.mid(first);
        }

        for (const ClassNode *node : path)
            cyclePath->append(node->name());
    }

    return cycleFound;
}

/*!
  Recursive helper for cycle detection using three-color DFS.

  Colors represent node states:
  \list
      \li White: Unvisited.
      \li Gray: Currently being processed (on recursion stack).
      \li Black: Fully processed (all descendants explored).
  \endlist

  Returns true if a cycle is detected. The \a path list will contain
  the nodes that form the cycle.

  \sa hasCircularRelationship().
*/
bool ClassNode::detectCycleRecursive(HierarchyDirection direction,
                                     QMap<const ClassNode*, Color> &colors,
                                     QList<const ClassNode*> &path) const
{
    colors[this] = Color::Gray;
    path.append(this);

    const QList<RelatedClass> &related = (direction == HierarchyDirection::Base)
                                          ? baseClasses()
                                          : derivedClasses();

    for (const auto &relatedClass : related) {
        const ClassNode *rc = relatedClass.m_node;
        if (rc == nullptr)
            rc = QDocDatabase::qdocDB()->findClassNode(relatedClass.m_path);

        if (rc != nullptr && (rc->isPrivate() || rc->isInternal() || rc->isDontDocument())) {
            if (rc == this) {
                qCDebug(lcQdoc) << "Skipping self-reference (CRTP-like) in" << this->name();
                continue;
            }

            Color rcColor = colors.value(rc, Color::White);

            if (rcColor == Color::Gray) {
                path.append(rc);
                return true;
            }

            if (rcColor == Color::White) {
                if (rc->detectCycleRecursive(direction, colors, path))
                    return true;
            }
        }
    }

    colors[this] = Color::Black;
    path.removeLast();

    return false;
}

/*!
  Checks if this class has circular inheritance by traversing its
  base class hierarchy. Returns true if a cycle is detected.

  If \a cyclePath is provided, it will be filled with the names of
  classes forming the cycle (e.g., ["A", "B", "C", "A"]).

  This method is used to detect problematic inheritance patterns
  before performing operations like base class promotion that could
  infinite loop on circular hierarchies.

  \sa hasCircularDerivedClasses(), hasCircularRelationship().
 */
bool ClassNode::hasCircularInheritance(QStringList *cyclePath) const
{
    return hasCircularRelationship(HierarchyDirection::Base, cyclePath);
}

/*!
  Checks if this class has circular derived class relationships by
  traversing its derived class hierarchy. Returns true if a cycle is detected.

  If \a cyclePath is provided, it will be filled with the names of
  classes forming the cycle.

  This is similar to hasCircularInheritance() but checks the derived
  direction instead of the base direction.

  \sa hasCircularInheritance(), hasCircularRelationship.
 */
bool ClassNode::hasCircularDerivedClasses(QStringList *cyclePath) const
{
    return hasCircularRelationship(HierarchyDirection::Derived, cyclePath);
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
void ClassNode::promotePublicBases(const QList<RelatedClass> &bases)
{
    if (!bases.isEmpty()) {
        for (qsizetype i = bases.size() - 1; i >= 0; --i) {
            ClassNode *bc = bases.at(i).m_node;
            if (bc == nullptr)
                bc = QDocDatabase::qdocDB()->findClassNode(bases.at(i).m_path);
            if (bc != nullptr) {
                if (bc->isPrivate() || bc->isInternal())
                    promotePublicBases(bc->baseClasses());
                else
                    m_bases.append(bases.at(i));
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
    while (i < m_bases.size()) {
        ClassNode *bc = m_bases.at(i).m_node;
        if (bc == nullptr)
            bc = QDocDatabase::qdocDB()->findClassNode(m_bases.at(i).m_path);
        if (bc != nullptr
            && (bc->isPrivate() || bc->isInternal() || bc->isDontDocument()
                || found.contains(bc))) {
            RelatedClass rc = m_bases.at(i);
            m_bases.removeAt(i);
            m_ignoredBases.append(rc);
            QStringList cyclePath;
            if (bc->hasCircularInheritance(&cyclePath))
                bc->location().warning("Circular inheritance detected: %1"_L1.arg(cyclePath.join(" -> ")));
            else
                promotePublicBases(bc->baseClasses());
        } else {
            ++i;
        }
        found.insert(bc);
    }

    i = 0;
    while (i < m_derived.size()) {
        ClassNode *dc = m_derived.at(i).m_node;
        if (dc != nullptr && (dc->isPrivate() || dc->isInternal() || dc->isDontDocument())) {
            QStringList derivedCyclePath;
            if (dc->hasCircularDerivedClasses(&derivedCyclePath)) {
                dc->location().warning(QStringLiteral("Circular derived class relationship detected: %1").arg(derivedCyclePath.join(" -> ")));
                m_derived.removeAt(i);
            } else {
                m_derived.removeAt(i);
                const QList<RelatedClass> &dd = dc->derivedClasses();
                for (qsizetype j = dd.size() - 1; j >= 0; --j) {
                    // Skip CRTP self-references to avoid infinite loops
                    if (dd.at(j).m_node != dc) {
                        m_derived.insert(i, dd.at(j));
                    } else {
                        qCDebug(lcQdoc) << "Skipping CRTP self-reference in derived class promotion for"
                                        << dc->name();
                    }
                }
            }
        } else {
            ++i;
        }
    }
}

/*!
 */
void ClassNode::resolvePropertyOverriddenFromPtrs(PropertyNode *pn)
{
    for (const auto &baseClass : std::as_const(baseClasses())) {
        ClassNode *cn = baseClass.m_node;
        if (cn) {
            Node *n = cn->findNonfunctionChild(pn->name(), &Node::isProperty);
            if (n) {
                auto *baseProperty = static_cast<PropertyNode *>(n);
                cn->resolvePropertyOverriddenFromPtrs(baseProperty);
                pn->setOverriddenFrom(baseProperty);
            } else
                cn->resolvePropertyOverriddenFromPtrs(pn);
        }
    }
}

QT_END_NAMESPACE
