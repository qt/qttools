// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

bool isOverThousand(int n)
{
//! [integer literal with separator]
    if (n > 1'000)
        return true;
//! [integer literal with separator]
    return false;
}
