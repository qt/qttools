// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

template <typename T>
class CrossThing
{
public:
    friend void crossCheck(CrossThing &a, CrossThing &b) noexcept
    {
        (void)a;
        (void)b;
    }
};

// A documented class that hosts the crossCheck documentation through \relates,
// while the friend itself is declared in CrossThing.
class CrossTarget
{
public:
    CrossTarget() = default;
};
