// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// The hidden friend's key argument is its first class template parameter.
// A \fn command that reuses the same parameter names in reversed
// declaration order while keeping the key argument spelled with the name
// that now occupies a different position describes a different function,
// because the positional binding changed even though the spellings match.
template <typename A, typename B>
class BindingProbe
{
public:
    friend void bindingCheck(BindingProbe &, A) noexcept
    {
    }
};
