// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <cstddef>

// The seeded overload is a body-less friend declaration. Its definition
// lives out-of-line in hashable.cpp. No namespace-scope declaration of
// this overload exists in any header, so the function is reachable from
// user translation units only via argument-dependent lookup.
//
// The unseeded overload is an inline forwarding friend. It exists because
// non-defining friend declarations cannot carry default arguments, so the
// "seed = 0" convenience cannot be folded into the seeded declaration.

class Hashable
{
public:
    Hashable() = default;
    explicit Hashable(int version) : m_version(version) {}

    int version() const { return m_version; }

private:
    friend int qHash(Hashable key, int seed) noexcept;
    friend int qHash(Hashable key) noexcept { return qHash(key, int{0}); }

    int m_version = 0;
};

