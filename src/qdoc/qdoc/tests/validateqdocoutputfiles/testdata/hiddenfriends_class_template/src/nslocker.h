// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

namespace locker_ns
{

template <typename Mutex>
class NamespaceLocker
{
public:
    friend void namespaceSwap(NamespaceLocker &lhs, NamespaceLocker &rhs) noexcept
    {
        (void)lhs;
        (void)rhs;
    }
};

} // namespace locker_ns
