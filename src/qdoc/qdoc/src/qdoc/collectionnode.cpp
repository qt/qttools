// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "collectionnode.h"

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/*!
  \class CollectionNode
  \brief A class for holding the members of a collection of doc pages.
 */

/*!
    Appends \a node to the collection node's member list and
    updates the new member's status.

    \sa setMemberStatus()
 */
void CollectionNode::addMember(Node *node)
{
    if (!m_members.contains(node))
        m_members.append(node);
    setMemberStatus(node);
}

/*!
    \reimp

    Sets this collection node's \a status and propagates it to
    the collection's members.
*/
void CollectionNode::setStatus(Status status)
{
    Node::setStatus(status);
    std::for_each(m_members.begin(), m_members.end(),
                  [this](Node *m) { this->setMemberStatus(m); });
}

/*!
    Propagates the collection's status to \a member node.

    To avoid unexpected results, this propagation is done only if:

    \list
        \li Both the member and the collection are part of an API
            (that is, this collection is a module),
        \li the module was \e seen (documented in the current
            project), and
        \li the member node has not set a custom status.
    \endlist
*/
void CollectionNode::setMemberStatus(Node *member)
{
    if (!wasSeen())
        return;

    if (!isApiGenus(this->genus()) || !isApiGenus(member->genus()))
        return;

    if (member->status() != Status::Active)
        return;

    member->setStatus(this->status());
}

/*!
  Returns \c true if this collection node contains at least
  one namespace node.
 */
bool CollectionNode::hasNamespaces() const
{
    return std::any_of(m_members.cbegin(), m_members.cend(), [](const Node *member) {
        return member->isClassNode() && member->isInAPI();
    });
}

/*!
  Returns \c true if this collection node contains at least
  one class node.
 */
bool CollectionNode::hasClasses() const
{
    return std::any_of(m_members.cbegin(), m_members.cend(), [](const Node *member) {
        return member->isClassNode() && member->isInAPI();
    });
}

/*!
    \fn template <typename F> NodeMap CollectionNode::getMembers(const F &&predicate) const

    Returns a map containing this collection node's member nodes for which \c
    predicate(node) returns \c true. The \a predicate is a function or a
    lambda that takes a const Node pointer as an argument and returns a bool.
*/

/*!
    \fn NodeMap CollectionNode::getMembers(Node::NodeType type) const

    Returns a map containing this collection node's member nodes with
    a specified node \a type.
*/

/*!
  Returns the logical module version.
*/
QString CollectionNode::logicalModuleVersion() const
{
    QStringList version;
    version << m_logicalModuleVersionMajor << m_logicalModuleVersionMinor;
    version.removeAll(QString());
    return version.join(".");
}

/*!
  This function accepts the logical module \a info as a string
  list. If the logical module info contains the version number,
  it splits the version number on the '.' character to get the
  major and minor version numbers. Both major and minor version
  numbers should be provided, but the minor version number is
  not strictly necessary.
 */
void CollectionNode::setLogicalModuleInfo(const QStringList &info)
{
    m_logicalModuleName = info[0];
    if (info.size() > 1) {
        QStringList dotSplit = info[1].split(QLatin1Char('.'));
        m_logicalModuleVersionMajor = dotSplit[0];
        if (dotSplit.size() > 1)
            m_logicalModuleVersionMinor = dotSplit[1];
        else
            m_logicalModuleVersionMinor = "0";
    }
}

/*!
    \fn void CollectionNode::setState(const QString &state)
    \fn QString CollectionNode::state()

    Sets or gets a description of this module's state. For example,
    \e {"Technology Preview"}. This string is used when generating the
    module's documentation page and reference pages of the module's
    members.
*/

QT_END_NAMESPACE
