// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// The template parameters are spelled A and B in code, and the
// documentation reuses those same names in reversed declaration order.
// The argument bindings stay consistent with the source, so the hidden
// friend must match the renamed documentation.
template <typename A, typename B>
class ReuseStore
{
public:
    friend void reuseCheck(ReuseStore &a, A key) noexcept
    {
        (void)a;
        (void)key;
    }
};
