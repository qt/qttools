// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// The template parameter is spelled T in code; the documentation renames it
// to U.
template <typename T>
class RenamedThing
{
public:
    friend void resetThing(RenamedThing &lhs) noexcept
    {
        (void)lhs;
    }
};
