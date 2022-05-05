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
