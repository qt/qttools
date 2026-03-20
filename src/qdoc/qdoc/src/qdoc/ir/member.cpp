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

} // namespace IR

QT_END_NAMESPACE
