// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmltypenode.h"
#include "collectionnode.h"
#include "qdocdatabase.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QMultiMap<const Node *, Node *> QmlTypeNode::s_inheritedBy;

/*!
  Constructs a Qml type.

  The new node has the given \a parent, name \a name, and a specific node
  \a type. Valid types are NodeType::QmlType and NodeType::QmlValueType.
 */
QmlTypeNode::QmlTypeNode(Aggregate *parent, const QString &name, NodeType type)
    : Aggregate(type, parent, name)
{
    Q_ASSERT(type == NodeType::QmlType || type == NodeType::QmlValueType);
    setTitle(name);
}

/*!
  Clear the static maps so that subsequent runs don't try to use
  contents from a previous run.
 */
void QmlTypeNode::terminate()
{
    s_inheritedBy.clear();
}

/*!
  Record the fact that QML class \a base is inherited by
  QML class \a sub.
 */
void QmlTypeNode::addInheritedBy(const Node *base, Node *sub)
{
    if (sub->isInternal())
        return;
    if (!s_inheritedBy.contains(base, sub))
        s_inheritedBy.insert(base, sub);
}

/*!
  Loads the list \a subs with the nodes of all the subclasses of \a base.
 */
void QmlTypeNode::subclasses(const Node *base, NodeList &subs)
{
    subs.clear();
    if (s_inheritedBy.count(base) > 0) {
        subs = s_inheritedBy.values(base);
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
    if (m_qmlBaseNode) {
        result = m_qmlBaseNode->logicalModuleName() + "::" + m_qmlBaseNode->name();
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
    return (m_logicalModule ? m_logicalModule->logicalModuleName() : QString());
}

/*!
  If the QML type's QML module pointer is set, return the QML
  module version from the QML module node. Otherwise, return
  the empty string.
 */
QString QmlTypeNode::logicalModuleVersion() const
{
    return (m_logicalModule ? m_logicalModule->logicalModuleVersion() : QString());
}

/*!
  If the QML type's QML module pointer is set, return the QML
  module identifier from the QML module node. Otherwise, return
  the empty string.
 */
QString QmlTypeNode::logicalModuleIdentifier() const
{
    return (m_logicalModule ? m_logicalModule->logicalModuleIdentifier() : QString());
}

/*!
  Returns true if this QML type inherits \a type.
 */
bool QmlTypeNode::inherits(Aggregate *type)
{
    QmlTypeNode *qtn = qmlBaseNode();
    while (qtn != nullptr) {
        if (qtn == type)
            return true;
        qtn = qtn->qmlBaseNode();
    }
    return false;
}

/*!
  Recursively resolves the base node for this QML type when only the name of
  the base type is known.

  \a previousSearches is used for speeding up the process.
*/
void QmlTypeNode::resolveInheritance(NodeMap &previousSearches)
{
    if (m_qmlBaseNode || m_qmlBaseName.isEmpty())
        return;

    auto *base = static_cast<QmlTypeNode *>(previousSearches.value(m_qmlBaseName));
    if (!previousSearches.contains(m_qmlBaseName)) {
        for (const auto &imp : std::as_const(m_importList)) {
            base = QDocDatabase::qdocDB()->findQmlType(imp, m_qmlBaseName);
            if (base)
                break;
        }
        if (!base) {
            if (m_qmlBaseName.contains(':'))
                base = QDocDatabase::qdocDB()->findQmlType(m_qmlBaseName);
            else
                base = QDocDatabase::qdocDB()->findQmlType(QString(), m_qmlBaseName);
        }
        previousSearches.insert(m_qmlBaseName, base);
    }

    if (base) {
        if (base != this) {
            m_qmlBaseNode = base;
            QmlTypeNode::addInheritedBy(base, this);
            // Base types read from the index need resolving as they only have the name set
            if (base->isIndexNode())
                base->resolveInheritance(previousSearches);
        } else
            location().report(QStringLiteral("Type is its own base type: '%1'").arg(name()));
    }

    if (!base)
        location().report(QStringLiteral("Unknown base '%1' for QML type '%2'").arg(qmlBaseName(), name()));
}

/*!
    Checks and warns about problems with the inheritance of this QML type.
*/
void QmlTypeNode::checkInheritance()
{
    /* Use Floyd's cycle-finding algorithm (tortoise and hare) to detect base
       types that inherit from their descendants. */
    const QmlTypeNode *qtn = this;
    const QmlTypeNode *hare = qtn;

    // Record the previous type found by the hare for reporting.
    QmlTypeNode *previous;

    while (qtn && hare) {
        // Examine the base node.
        qtn = qtn->qmlBaseNode();

        /* The hare node moves two nodes up the inheritance tree to increase
           the cycle detection distance, recording the previous type in case
           it needs to be reported. */
        for (int i = 0; i < 2; i++)
            if (hare) {
                previous = const_cast<QmlTypeNode *>(hare);
                hare = hare->qmlBaseNode();
            }

        // Only report a cycle if both nodes are non-null and identical.
        if (qtn && hare && qtn == hare) {
            location().report(QStringLiteral("Circular type inheritance: '%1'").arg(previous->name()));
            previous->m_qmlBaseNode = nullptr;
            break;
        }
    }
}

QT_END_NAMESPACE
