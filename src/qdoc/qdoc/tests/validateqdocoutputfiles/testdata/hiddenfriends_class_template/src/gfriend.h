// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// A non-template class with a friend that is itself a genuine function
// template. The friend must keep its own template classification and must
// not receive enclosing-class fallback treatment.
class GHolder
{
public:
    template <typename Other>
    friend void gfriend(GHolder &a, Other &b) noexcept
    {
        (void)a;
        (void)b;
    }
};
