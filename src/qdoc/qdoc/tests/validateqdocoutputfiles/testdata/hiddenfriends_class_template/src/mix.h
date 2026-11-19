// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// A genuine free function template with the same name as the hidden friend
// of Mixture. Its broad parameter list must not prevent resolution of the
// hidden friend, and an exact match must still resolve to this template.
template <typename A, typename U>
void mix(A &a, U &b) noexcept
{
    (void)a;
    (void)b;
}

template <typename M>
class Mixture
{
public:
    friend void mix(Mixture &a, Mixture &b) noexcept
    {
        (void)a;
        (void)b;
    }
};
