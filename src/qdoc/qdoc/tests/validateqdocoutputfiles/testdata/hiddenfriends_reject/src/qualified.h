// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

namespace ConcreteTypes
{
struct T {};
struct U {};
}

// The hidden friend takes a fixed qualified type whose final component
// shares a name with the class's template parameter. The two fixed types
// are distinct, so a \fn command that renames the parameter cannot match
// the friend: the qualified name is not a reference to the parameter.
template <typename T>
class QualifiedProbe
{
public:
    friend void qualifiedCheck(QualifiedProbe &, ConcreteTypes::T &) noexcept
    {
    }
};
