// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

template <typename T>
class Handle
{
public:
    T value;

    friend constexpr bool operator==(const Handle &lhs, const Handle &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }
};

class Session
{
public:
    friend bool operator==(const Session &lhs, const Session &rhs);
    friend bool operator!=(const Session &lhs, const Session &rhs) { return !(lhs == rhs); }
};

// Evaluating this instantiates the hidden friend of Handle<int>, which Clang
// then reports as a top-level declaration named operator==.
static_assert(Handle<int>{1} == Handle<int>{1});
