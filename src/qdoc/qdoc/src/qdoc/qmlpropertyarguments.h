// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLPROPERTYARGUMENTS_H
#define QMLPROPERTYARGUMENTS_H

#include <QtCore/qstring.h>
#include <optional>
#include <type_traits>

QT_BEGIN_NAMESPACE

class Location;

struct QmlPropertyArguments
{
    enum class ParsingOptions {
        None = 0x0,
        RequireQualifiedPath = 0x1,
        IgnoreType = 0x2
    };

    QString m_type {};
    QString m_module {};
    QString m_qmltype {};
    QString m_name {};
    bool m_isList { false };

    static std::optional<QmlPropertyArguments>
    parse(const QString &arg, const Location &loc,
          ParsingOptions opts = ParsingOptions::None);

    friend ParsingOptions operator&(ParsingOptions lhs, ParsingOptions rhs)
    {
        return static_cast<ParsingOptions>(
            static_cast<std::underlying_type<ParsingOptions>::type>(lhs) &
            static_cast<std::underlying_type<ParsingOptions>::type>(rhs)
        );
    }

    friend ParsingOptions operator|(ParsingOptions lhs, ParsingOptions rhs)
    {
        return static_cast<ParsingOptions>(
            static_cast<std::underlying_type<ParsingOptions>::type>(lhs) |
            static_cast<std::underlying_type<ParsingOptions>::type>(rhs)
        );
    }

};

QT_END_NAMESPACE

#endif // QMLPROPERTYARGUMENTS_H
