// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "variablenode.h"

QT_BEGIN_NAMESPACE

/*!
  Clone this node on the heap and make the clone a child of
  \a parent.

  Returns a pointer to the clone.
 */
Node *VariableNode::clone(Aggregate *parent)
{
    auto *vn = new VariableNode(*this); // shallow copy
    vn->setParent(nullptr);
    parent->addChild(vn);

    return vn;
}

QT_END_NAMESPACE
