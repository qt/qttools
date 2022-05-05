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

#ifndef VARIABLENODE_H
#define VARIABLENODE_H

#include "aggregate.h"
#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class VariableNode : public Node
{
public:
    VariableNode(Aggregate *parent, const QString &name);

    void setLeftType(const QString &leftType) { m_leftType = leftType; }
    void setRightType(const QString &rightType) { m_rightType = rightType; }
    void setStatic(bool b) { m_static = b; }

    [[nodiscard]] const QString &leftType() const { return m_leftType; }
    [[nodiscard]] const QString &rightType() const { return m_rightType; }
    [[nodiscard]] QString dataType() const { return m_leftType + m_rightType; }
    [[nodiscard]] bool isStatic() const override { return m_static; }
    Node *clone(Aggregate *parent) override;

private:
    QString m_leftType {};
    QString m_rightType {};
    bool m_static { false };
};

inline VariableNode::VariableNode(Aggregate *parent, const QString &name)
    : Node(Variable, parent, name)
{
    setGenus(Node::CPP);
}

QT_END_NAMESPACE

#endif // VARIABLENODE_H
