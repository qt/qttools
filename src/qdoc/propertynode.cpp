// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "propertynode.h"

#include "aggregate.h"

QT_BEGIN_NAMESPACE

/*!
  \class PropertyNode

  This class describes one instance of using the Q_PROPERTY macro.
 */

/*!
  The constructor sets the \a parent and the \a name, but
  everything else is left to default values.
 */
PropertyNode::PropertyNode(Aggregate *parent, const QString &name) : Node(Property, parent, name)
{
    // nothing
}

/*!
  Sets this property's \e {overridden from} property to
  \a baseProperty, which indicates that this property
  overrides \a baseProperty. To begin with, all the values
  in this property are set to the corresponding values in
  \a baseProperty.

  We probably should ensure that the constant and final
  attributes are not being overridden improperly.
 */
void PropertyNode::setOverriddenFrom(const PropertyNode *baseProperty)
{
    for (int i = 0; i < NumFunctionRoles; ++i) {
        if (m_functions[i].isEmpty())
            m_functions[i] = baseProperty->m_functions[i];
    }
    if (m_stored == FlagValueDefault)
        m_stored = baseProperty->m_stored;
    if (m_designable == FlagValueDefault)
        m_designable = baseProperty->m_designable;
    if (m_scriptable == FlagValueDefault)
        m_scriptable = baseProperty->m_scriptable;
    if (m_writable == FlagValueDefault)
        m_writable = baseProperty->m_writable;
    if (m_user == FlagValueDefault)
        m_user = baseProperty->m_user;
    m_overrides = baseProperty;
}

/*!
  Returns a string containing the data type qualified with "const" either
  prepended to the data type or appended to it, or without the const
  qualification, depending circumstances in the PropertyNode internal state.
 */
QString PropertyNode::qualifiedDataType() const
{
    if (m_propertyType != Standard || m_type.startsWith(QLatin1String("const ")))
        return m_type;

    if (setters().isEmpty() && resetters().isEmpty()) {
        if (m_type.contains(QLatin1Char('*')) || m_type.contains(QLatin1Char('&'))) {
            // 'QWidget *' becomes 'QWidget *' const
            return m_type + " const";
        } else {
            /*
              'int' becomes 'const int' ('int const' is
              correct C++, but looks wrong)
             */
            return "const " + m_type;
        }
    } else {
        return m_type;
    }
}

/*!
  Returns true if this property has an access function named \a name.
 */
bool PropertyNode::hasAccessFunction(const QString &name) const
{
    for (const auto &getter : getters()) {
        if (getter->name() == name)
            return true;
    }
    for (const auto &setter : setters()) {
        if (setter->name() == name)
            return true;
    }
    for (const auto &resetter : resetters()) {
        if (resetter->name() == name)
            return true;
    }
    for (const auto &notifier : notifiers()) {
        if (notifier->name() == name)
            return true;
    }
    return false;
}

/*!
  Returns true if function \a functionNode has role \a r for this
  property.
 */
PropertyNode::FunctionRole PropertyNode::role(const FunctionNode *functionNode) const
{
    for (int i = 0; i < 4; i++) {
        if (m_functions[i].contains(const_cast<FunctionNode *>(functionNode)))
            return (FunctionRole)i;
    }
    return Notifier;
}

QT_END_NAMESPACE
