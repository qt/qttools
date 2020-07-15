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

#ifndef QMLPROPERTYNODE_H
#define QMLPROPERTYNODE_H

#include "aggregate.h"
#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QmlPropertyNode : public Node
{
public:
    QmlPropertyNode(Aggregate *parent, const QString &name, const QString &type, bool attached);

    void setDataType(const QString &dataType) override { m_type = dataType; }
    void setStored(bool stored) { m_stored = toFlagValue(stored); }
    void setDesignable(bool designable) { m_designable = toFlagValue(designable); }
    void setRequired() { m_required = toFlagValue(true); }

    const QString &dataType() const { return m_type; }
    QString qualifiedDataType() const { return m_type; }
    bool isReadOnlySet() const { return (readOnly_ != FlagValueDefault); }
    bool isStored() const { return fromFlagValue(m_stored, true); }
    bool isDesignable() const { return fromFlagValue(m_designable, false); }
    bool isWritable();
    bool isRequired();
    bool isDefault() const override { return m_isDefault; }
    bool isReadOnly() const override { return fromFlagValue(readOnly_, false); }
    bool isAlias() const override { return m_isAlias; }
    bool isAttached() const override { return m_attached; }
    bool isQtQuickNode() const override { return parent()->isQtQuickNode(); }
    QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    QString logicalModuleName() const override { return parent()->logicalModuleName(); }
    QString logicalModuleVersion() const override { return parent()->logicalModuleVersion(); }
    QString logicalModuleIdentifier() const override { return parent()->logicalModuleIdentifier(); }
    QString element() const override { return parent()->name(); }

    void markDefault() override { m_isDefault = true; }
    void markReadOnly(bool flag) override { readOnly_ = toFlagValue(flag); }

private:
    PropertyNode *findCorrespondingCppProperty();

private:
    QString m_type {};
    FlagValue m_stored { FlagValueDefault };
    FlagValue m_designable { FlagValueDefault };
    bool m_isAlias { false };
    bool m_isDefault { false };
    bool m_attached {};
    FlagValue readOnly_ { FlagValueDefault };
    FlagValue m_required { FlagValueDefault };
};

QT_END_NAMESPACE

#endif // QMLPROPERTYNODE_H
