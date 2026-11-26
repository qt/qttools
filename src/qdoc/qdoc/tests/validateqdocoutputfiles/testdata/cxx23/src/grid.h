// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

class Grid
{
public:
    int operator[](int row) const;
#ifdef __cpp_multidimensional_subscript
    int operator[](int row, int column) const;
#endif
};
