// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "typedefnode.h"

#include "aggregate.h"

QT_BEGIN_NAMESPACE

/*!
  \class TypedefNode
 */

/*!
 */
void TypedefNode::setAssociatedEnum(const EnumNode *enume)
{
    m_associatedEnum = enume;
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent.

  Returns the pointer to the clone.
 */
Node *TypedefNode::clone(Aggregate *parent)
{
    auto *tn = new TypedefNode(*this); // shallow copy
    tn->setParent(nullptr);
    parent->addChild(tn);

    return tn;
}

/*!
  \class TypeAliasNode
 */

/*!
  Clone this node on the heap and make the clone a child of
  \a parent.

  Returns the pointer to the clone.
 */
Node *TypeAliasNode::clone(Aggregate *parent)
{
    auto *tan = new TypeAliasNode(*this); // shallow copy
    tan->setParent(nullptr);
    parent->addChild(tan);

    return tan;
}

QT_END_NAMESPACE
