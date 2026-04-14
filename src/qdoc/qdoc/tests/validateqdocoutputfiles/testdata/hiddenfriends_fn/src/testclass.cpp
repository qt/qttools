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

// QTBUG-145790: Hidden friend with \relates targeting a different class,
// plus a template overload with the same name.

/*!
    \class Container
    \inmodule HiddenFriendModule
    \brief A container class that collects related non-member functions.
*/

/*!
    \class MyClass
    \inmodule HiddenFriendModule
    \brief A class with a hidden friend that should be documented on Container's page.
*/

/*!
    \fn template <typename T> bool myEquals(const T &a, const T &b)
    \relates Container

    Generic implementation. Returns \c true if \a a equals \a b.
*/

/*!
    \fn bool myEquals(const MyClass &a, const MyClass &b) noexcept
    \relates Container

    MyClass-specific overload. Returns \c true if \a a and \a b
    have the same internal value.
*/
