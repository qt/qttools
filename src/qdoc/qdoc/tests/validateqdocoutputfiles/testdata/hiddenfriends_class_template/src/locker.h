// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// Named PlainLocker (not QMutexLocker) on purpose: the test harness adds
// Qt's own include paths, and a class named QMutexLocker would resolve its
// include file to the real Qt convenience header of that name.
template <typename Mutex>
class PlainLocker
{
public:
    void swap(PlainLocker &other) noexcept {}

    // Non-template hidden friend. Each instantiation of the class template
    // carries its own swap, but the friend itself is not a function
    // template. Documented out-of-line in locker.qdoc with a leading
    // template declaration that names the enclosing class's parameter.
    friend void swap(PlainLocker &lhs, PlainLocker &rhs) noexcept
    {
        lhs.swap(rhs);
    }
};
