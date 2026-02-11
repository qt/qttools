// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlpropertyarguments.h"
#include "location.h"

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
    \class QmlPropertyArguments
    \brief Helper class for parsing QML property and QML method arguments.
*/

/*!
    \enum QmlPropertyArguments::ParsingOptions

    \value None
           No options specified.
    \value RequireQualifiedPath
           Require the path segment to be qualified with a
           QML type name.
    \value IgnoreType
           Don’t require a type segment when parsing.
    \value ParseAsMethod
           Parse input as a method, expecting to see also
           QML method/signal parameters.

    \sa parse()
*/

/*!
    Parses a QML property from the input string \a str, with parsing options
    \a opts. \a str contains the argument string passed to e.g. the
    \\qmlproperty command.

    A valid QML property has the following syntax:

    \badcode
    <type> <QML-type>::<name>
    <type> <QML-module>::<QML-type>::<name>
    \endcode

    In addition, if parsing option RequireQualifiedPath is \b not set, the
    following is also accepted:

    \badcode
    <type> <name>
    \endcode

    \e {<type>} can be omitted if \c IgnoreType option is set.

    This syntax is accepted in QmlDocVisitor where the associated QML type
    is already known.

    If \c ParseAsMethod option is set, a parameter list enclosed in
    parentheses must appear at the end. An empty list, i.e. \e {()}, is accepted.

    Returns a populated QmlPropertyArguments container with the property type
    (m_type), whether the type is a list type (m_isList), property name
    (m_name), and optionally, the parent QML type name (m_qmltype) and QML
    module name (m_module) if those were present in the argument string.
    For methods, the parameter list (m_params) is also set.

    If the argument string is incorrect, outputs a warning using \a loc and
    returns \c nullopt.
*/
std::optional<QmlPropertyArguments>
QmlPropertyArguments::parse(const QString &str, const Location &loc, ParsingOptions opts)
{
    // Special case: an empty string received from DocParser due to syntax error
    // Assumes a warning was issued already.
    if (str.isEmpty())
        return std::nullopt;

    auto input{str.trimmed()};
    QmlPropertyArguments args;

    bool is_method = (opts & ParsingOptions::ParseAsMethod) != ParsingOptions::None;

    auto paramStart = input.indexOf('('_L1);
    if (paramStart != -1) {
        if (auto paramEnd = input.indexOf(')'_L1, ++paramStart); paramEnd != -1)
            args.m_params = input.sliced(paramStart, paramEnd - paramStart).trimmed();
        input.resize(paramStart - 1);
        input = input.trimmed();
    }
    if (is_method && args.m_params.isNull()) {
        loc.warning("Invalid syntax for \\qmlmethod: %1"_L1.arg(str));
        return std::nullopt;
    }

    auto offset = input.indexOf(' '_L1);
    if (offset == -1) {
        if ((opts & ParsingOptions::IgnoreType) == ParsingOptions::None) {
            loc.warning("Missing %1 type for %2"_L1
                    .arg(is_method ? "return"_L1 : "property"_L1, str));
            return std::nullopt;
        }
    } else {
        args.m_type = input.first(offset);
        if ((args.m_isList = args.m_type.startsWith("list<"_L1)) && !is_method)
            args.m_type.slice(5).chop(1);
    }

    auto segments = input.slice(++offset).split("::"_L1);
    if (segments.size() > 3 || (segments.size() == 1 &&
            (opts & ParsingOptions::RequireQualifiedPath) != ParsingOptions::None)) {
        loc.warning("Unrecognizable QML module/type qualifier for %1"_L1.arg(str));
        return std::nullopt;
    }
    args.m_name = segments.takeLast();
    if (segments.size() > 0)
        args.m_qmltype = segments.takeLast();
    if (segments.size() > 0)
        args.m_module = segments[0];

    return std::optional(args);
}

QT_END_NAMESPACE
