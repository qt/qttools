/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
