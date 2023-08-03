// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
