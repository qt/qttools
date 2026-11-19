// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

template <typename Mutex>
class QualLocker
{
public:
    friend void qswap(QualLocker &lhs, QualLocker &rhs) noexcept
    {
        (void)lhs;
        (void)rhs;
    }
};
