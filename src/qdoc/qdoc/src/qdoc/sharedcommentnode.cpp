// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
