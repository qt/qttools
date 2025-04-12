// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef VARIABLENODE_H
#define VARIABLENODE_H

#include "aggregate.h"
#include "genustypes.h"
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
    : Node(NodeType::Variable, parent, name)
{
    setGenus(Genus::CPP);
}

QT_END_NAMESPACE

#endif // VARIABLENODE_H
