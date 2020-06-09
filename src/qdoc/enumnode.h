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

private:
    QList<EnumItem> m_items {};
    QSet<QString> m_names {};
    const TypedefNode *m_flagsType { nullptr };
    bool m_isScoped { false };
};

QT_END_NAMESPACE

#endif // ENUMNODE_H
