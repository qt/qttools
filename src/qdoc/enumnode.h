// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ENUMNODE_H
#define ENUMNODE_H

#include "access.h"
#include "node.h"
#include "typedefnode.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Aggregate;

class EnumNode : public Node
{
public:
    EnumNode(Aggregate *parent, const QString &name, bool isScoped = false)
        : Node(Enum, parent, name), m_isScoped(isScoped)
    {
    }

    void addItem(const EnumItem &item);
    void setFlagsType(TypedefNode *typedefNode);
    bool hasItem(const QString &name) const { return m_names.contains(name); }
    bool isScoped() const { return m_isScoped; }

    const QList<EnumItem> &items() const { return m_items; }
    Access itemAccess(const QString &name) const;
    const TypedefNode *flagsType() const { return m_flagsType; }
    QString itemValue(const QString &name) const;
    Node *clone(Aggregate *parent) override;
    void setSince(const QString &value, const QString &since);

private:
    QList<EnumItem> m_items {};
    QSet<QString> m_names {};
    const TypedefNode *m_flagsType { nullptr };
    bool m_isScoped { false };
};

QT_END_NAMESPACE

#endif // ENUMNODE_H
