// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "hashable.h"

int qHash(Hashable key, int seed) noexcept
{
    return static_cast<int>(key.version()) ^ seed;
}

/*!
    \class Hashable
    \inmodule HiddenFriendModule
    \brief A class with an out-of-line-defined hidden friend qHash overload.

    Exercises QDoc's resolution of an unqualified \\fn command for a friend
    declaration that carries no inline body and whose definition lives in a
    separate translation unit. Mirrors the QShaderVersion qHash pattern
    documented under QTBUG-144141.
*/

/*!
    \fn int qHash(Hashable key, int seed)

    Returns the hash value for \a key, using \a seed to seed the calculation.

    The friend declaration in hashable.h carries no body; this definition
    lives in hashable.cpp. QDoc must recognize this as a hidden friend even
    though no inline body appears in the class.
*/

/*!
    \fn int qHash(Hashable key)

    Returns the hash value for \a key.

    Equivalent to calling \c{qHash(key, 0)}. Inline forwarding overload
    co-located with the seeded form.
*/

