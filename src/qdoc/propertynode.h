// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROPERTYNODE_H
#define PROPERTYNODE_H

#include "functionnode.h"
#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Aggregate;

class PropertyNode : public Node
{
public:
    enum class PropertyType { StandardProperty, BindableProperty };
    enum class FunctionRole { Getter, Setter, Resetter, Notifier, Bindable, NumFunctionRoles };
    static QString roleName(FunctionRole role);

    PropertyNode(Aggregate *parent, const QString &name);

    void setDataType(const QString &dataType) override { m_type = dataType; }
    void addFunction(FunctionNode *function, FunctionRole role);
    void addSignal(FunctionNode *function, FunctionRole role);
    void setStored(bool stored) { m_stored = toFlagValue(stored); }
    void setWritable(bool writable) { m_writable = toFlagValue(writable); }
    void setOverriddenFrom(const PropertyNode *baseProperty);
    void setConstant() { m_const = true; }
    void setRequired() { m_required = true; }
    void setPropertyType(PropertyType type) { m_propertyType = type; }

    [[nodiscard]] const QString &dataType() const { return m_type; }
    [[nodiscard]] QString qualifiedDataType() const;
    [[nodiscard]] NodeList functions() const;
    [[nodiscard]] const NodeList &functions(FunctionRole role) const
    {
        return m_functions[(int)role];
    }
    [[nodiscard]] const NodeList &getters() const { return functions(FunctionRole::Getter); }
    [[nodiscard]] const NodeList &setters() const { return functions(FunctionRole::Setter); }
    [[nodiscard]] const NodeList &resetters() const { return functions(FunctionRole::Resetter); }
    [[nodiscard]] const NodeList &notifiers() const { return functions(FunctionRole::Notifier); }
    [[nodiscard]] bool hasAccessFunction(const QString &name) const;
    FunctionRole role(const FunctionNode *functionNode) const;
    [[nodiscard]] bool isStored() const { return fromFlagValue(m_stored, storedDefault()); }
    [[nodiscard]] bool isWritable() const { return fromFlagValue(m_writable, writableDefault()); }
    [[nodiscard]] bool isConstant() const { return m_const; }
    [[nodiscard]] bool isRequired() const { return m_required; }
    [[nodiscard]] PropertyType propertyType() const { return m_propertyType; }
    [[nodiscard]] const PropertyNode *overriddenFrom() const { return m_overrides; }

    [[nodiscard]] bool storedDefault() const { return true; }
    [[nodiscard]] bool writableDefault() const { return !setters().isEmpty(); }

private:
    QString m_type {};
    PropertyType m_propertyType { PropertyType::StandardProperty };
    NodeList m_functions[(qsizetype)FunctionRole::NumFunctionRoles] {};
    FlagValue m_stored { FlagValueDefault };
    FlagValue m_writable { FlagValueDefault };
    FlagValue m_user { FlagValueDefault };
    bool m_const { false };
    bool m_required { false };
    const PropertyNode *m_overrides { nullptr };
};

inline void PropertyNode::addFunction(FunctionNode *function, FunctionRole role)
{
    m_functions[(int)role].append(function);
    function->addAssociatedProperty(this);
}

inline void PropertyNode::addSignal(FunctionNode *function, FunctionRole role)
{
    m_functions[(int)role].append(function);
    function->addAssociatedProperty(this);
}

inline NodeList PropertyNode::functions() const
{
    NodeList list;
    for (qsizetype i{0}; i < (qsizetype)FunctionRole::NumFunctionRoles; ++i)
        list += m_functions[i];
    return list;
}

QT_END_NAMESPACE

#endif // PROPERTYNODE_H
