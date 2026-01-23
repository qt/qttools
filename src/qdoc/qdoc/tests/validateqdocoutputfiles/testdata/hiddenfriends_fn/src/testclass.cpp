// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testclass.h"

/*!
    \class TestClass
    \inmodule HiddenFriendModule
    \brief A class with hidden friends documented using unqualified fn syntax.

    This class tests that hidden friends can be documented using fn commands
    without requiring the class prefix.
*/

/*!
    \fn bool operator==(const TestClass &lhs, const TestClass &rhs)
    \relates TestClass

    Hidden friend equality operator documented without class prefix.

    Compares \a lhs and \a rhs for equality.
    Returns \c true if they are equal, \c false otherwise.
*/

/*!
    \fn bool operator<(const TestClass &lhs, const TestClass &rhs)
    \relates TestClass

    Hidden friend less-than operator documented without class prefix.

    Compares \a lhs and \a rhs.
    Returns \c true if \a lhs is less than \a rhs, \c false otherwise.
*/

