/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "sharedcommentnode.h"

#include "aggregate.h"
#include "functionnode.h"
#include "qmltypenode.h"

QT_BEGIN_NAMESPACE

SharedCommentNode::SharedCommentNode(QmlTypeNode *parent, int count, QString &group)
    : Node(Node::SharedComment, parent, group)
{
    m_collective.reserve(count);
}

/*!
  Searches the shared comment node's member nodes for function
  nodes. Each function node's overload flag is set.
 */
void SharedCommentNode::setOverloadFlags()
{
    for (auto *node : m_collective) {
        if (node->isFunction())
            static_cast<FunctionNode *>(node)->setOverloadFlag();
    }
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent.

  Returns the pointer to the clone.
 */
Node *SharedCommentNode::clone(Aggregate *parent)
{
    auto *scn = new SharedCommentNode(*this); // shallow copy
    scn->setParent(nullptr);
    parent->addChild(scn);

    return scn;
}

/*!
  Sets the related nonmember flag in this node and in each
  node in the shared comment's collective to \a value.
 */
void SharedCommentNode::setRelatedNonmember(bool value)
{
    Node::setRelatedNonmember(value);
    for (auto *node : m_collective)
        node->setRelatedNonmember(value);
}

QT_END_NAMESPACE
