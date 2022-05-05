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

#ifndef TYPEDEFNODE_H
#define TYPEDEFNODE_H

#include "enumnode.h"
#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Aggregate;

class TypedefNode : public Node
{
public:
    TypedefNode(Aggregate *parent, const QString &name, NodeType type = Typedef)
        : Node(type, parent, name)
    {
    }

    bool hasAssociatedEnum() const { return m_associatedEnum != nullptr; }
    const EnumNode *associatedEnum() const { return m_associatedEnum; }
    Node *clone(Aggregate *parent) override;

private:
    void setAssociatedEnum(const EnumNode *t);

    friend class EnumNode;

    const EnumNode *m_associatedEnum { nullptr };
};

class TypeAliasNode : public TypedefNode
{
public:
    TypeAliasNode(Aggregate *parent, const QString &name, const QString &aliasedType)
        : TypedefNode(parent, name, NodeType::TypeAlias), m_aliasedType(aliasedType)
    {
    }

    const QString &aliasedType() const { return m_aliasedType; }
    Node *clone(Aggregate *parent) override;

private:
    QString m_aliasedType {};
};

QT_END_NAMESPACE

#endif // TYPEDEFNODE_H
