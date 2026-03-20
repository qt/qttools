// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "member.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \struct IR::ParameterIR
    \brief Intermediate representation of a function parameter.

    ParameterIR captures the type, name, and optional default value
    of a single function parameter. Templates use this to render
    parameter lists in function synopses.

    JSON output omits \c defaultValue when the string is empty,
    following the convention of suppressing absent optional fields.
*/

/*!
    \variable IR::ParameterIR::type
    Parameter type (such as "const QString &").
*/

/*!
    \variable IR::ParameterIR::name
    Parameter name.
*/

/*!
    \variable IR::ParameterIR::defaultValue
    Default value expression, empty if none.
*/

/*!
    Converts the parameter to a QJsonObject for template rendering.

    Always emits \c type and \c name. The \c defaultValue key is
    omitted when the default value string is empty.
*/
QJsonObject ParameterIR::toJson() const
{
    QJsonObject json;
    json["type"_L1] = type;
    json["name"_L1] = name;
    if (!defaultValue.isEmpty())
        json["defaultValue"_L1] = defaultValue;
    return json;
}

/*!
    \struct IR::EnumValueIR
    \brief Intermediate representation of a single enum value.

    EnumValueIR captures the name, explicit initializer, and version
    information for one enumerator. Templates use this to render
    enum value tables in class documentation.

    JSON output omits \c value and \c since when their respective
    strings are empty.
*/

/*!
    \variable IR::EnumValueIR::name
    Enumerator name.
*/

/*!
    \variable IR::EnumValueIR::value
    Explicit initializer expression, empty if the compiler assigns
    the value.
*/

/*!
    \variable IR::EnumValueIR::since
    Qt version that introduced this enumerator, empty if unversioned.
*/

/*!
    Converts the enum value to a QJsonObject for template rendering.

    Always emits \c name. The \c value and \c since keys are omitted
    when their respective strings are empty.
*/
QJsonObject EnumValueIR::toJson() const
{
    QJsonObject json;
    json["name"_L1] = name;
    if (!value.isEmpty())
        json["value"_L1] = value;
    if (!since.isEmpty())
        json["since"_L1] = since;
    return json;
}

/*!
    \struct IR::InheritedMembersIR
    \brief Summary of members inherited from a single base class.

    InheritedMembersIR stores a count and link target for one base
    class's contributed members. Templates use this to render lines
    such as "5 public functions inherited from QObject" at the end
    of a section.
*/

/*!
    \variable IR::InheritedMembersIR::className
    Fully qualified name of the base class.
*/

/*!
    \variable IR::InheritedMembersIR::count
    Number of members inherited from this base class.
*/

/*!
    \variable IR::InheritedMembersIR::href
    URL of the base class's documentation page.
*/

/*!
    Converts the inherited members summary to a QJsonObject.

    Emits \c className, \c count, and \c href. Templates combine
    this with the enclosing section's \c plural field to render
    links such as "5 public functions inherited from QObject".
*/
QJsonObject InheritedMembersIR::toJson() const
{
    QJsonObject json;
    json["className"_L1] = className;
    json["count"_L1] = count;
    json["href"_L1] = href;
    return json;
}

} // namespace IR

QT_END_NAMESPACE
