// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// Two type parameters; the hidden friend binds them asymmetrically, so a
// documentation with reversed argument bindings must not match it.
template <typename Lhs, typename Rhs>
class DualStore
{
public:
    friend void dualCheck(DualStore &a, Lhs key) noexcept
    {
        (void)a;
        (void)key;
    }
};
