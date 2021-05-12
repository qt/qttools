/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
