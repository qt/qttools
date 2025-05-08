// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLPROPERTYNODE_H
#define QMLPROPERTYNODE_H

#include "aggregate.h"
#include "nativeenum.h"
#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QmlPropertyNode : public Node, public NativeEnumInterface
{
public:
    QmlPropertyNode(Aggregate *parent, const QString &name, QString type, bool attached);

    void setDataType(const QString &dataType) override;
    void setStored(bool stored) { m_stored = toFlagValue(stored); }
    void setDefaultValue(const QString &value) { m_defaultValue = value; }
    void setRequired() { m_required = toFlagValue(true); }
    void setIsList(bool isList);

    [[nodiscard]] const QString &dataType() const { return m_type; }
    [[nodiscard]] bool validateDataType(const QString &type = QString()) const;
    [[nodiscard]] const QString &defaultValue() const { return m_defaultValue; }
    [[nodiscard]] bool isStored() const { return fromFlagValue(m_stored, true); }
    bool isRequired();
    [[nodiscard]] bool isDefault() const override { return m_isDefault; }
    [[nodiscard]] bool isReadOnly() const { return fromFlagValue(m_readOnly, false); }
    [[nodiscard]] bool isReadOnly();
    [[nodiscard]] bool isAlias() const override { return m_isAlias; }
    [[nodiscard]] bool isAttached() const override { return m_attached; }
    [[nodiscard]] QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    [[nodiscard]] QString logicalModuleName() const override
    {
        return parent()->logicalModuleName();
    }
    [[nodiscard]] QString logicalModuleVersion() const override
    {
        return parent()->logicalModuleVersion();
    }
    [[nodiscard]] QString logicalModuleIdentifier() const override
    {
        return parent()->logicalModuleIdentifier();
    }
    [[nodiscard]] QString element() const override { return parent()->name(); }
    NativeEnum *nativeEnum() override { return &m_nativeEnum; }
    const NativeEnum *nativeEnum() const override { return &m_nativeEnum; }

    void markDefault() override { m_isDefault = true; }
    void markReadOnly(bool flag) override { m_readOnly = toFlagValue(flag); }

private:
    PropertyNode *findCorrespondingCppProperty();

private:
    QString m_type {};
    QString m_defaultValue {};
    FlagValue m_stored { FlagValueDefault };
    bool m_isAlias { false };
    bool m_isDefault { false };
    bool m_attached {};
    FlagValue m_isList { FlagValueDefault };
    FlagValue m_readOnly { FlagValueDefault };
    FlagValue m_required { FlagValueDefault };
    NativeEnum m_nativeEnum;
    static QSet<QString> cppQmlValueTypes;
    static QRegularExpression cppBasicList;
    static QRegularExpression qmlBasicList;
};

QT_END_NAMESPACE

#endif // QMLPROPERTYNODE_H
