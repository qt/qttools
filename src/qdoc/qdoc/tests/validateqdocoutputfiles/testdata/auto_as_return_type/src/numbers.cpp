// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "numbers.h"

/*!
    \module QDocTest

    \title QDocTest Module
    The QDocTest module provides classes for testing QDoc.
 */

/*!
    \namespace QDocTests
    \brief The QDocTests namespace contains classes for testing QDoc.
    \inmodule QDocTest
*/

/*!
    \class QDocTests::Numbers
    \inmodule QDocTest
    \brief The Numbers class provides functions for manipulating numbers.
*/

namespace QDocTests {

/*!
    \brief Adds two numbers.

    Returns an integer containing the sum of \a a and \a b.
*/
auto Numbers::add(int a, int b)
{
    return a + b;
}

/*!
    \fn template <typename T> T QDocTests::Numbers::add(T a, T b)
    \brief Adds two numbers of the same underlying type.

    Returns a value containing the sum of \a a and \a b.
*/

/*!
    \fn template <typename ThisIsMyReallyLongAndComplicatedTemplateName> auto QDocTests::Numbers::adder(ThisIsMyReallyLongAndComplicatedTemplateName a, int b)
    \brief Adds two numbers of the same underlying type.

    Returns a value containing the sum of \a a and \a b.
*/

/*!
    \fn auto QDocTests::Numbers::subtract(int a, int b)
    \brief Subtracts two numbers.

    Returns an integer containing the difference of \a a and \a b.
*/

/*!
    \fn auto QDocTests::Numbers::number() const
    \brief Returns the number.

    Returns the number, whatever it may be.
*/

/*!
    \fn void QDocTests::Numbers::setNumber(int number)
    \brief Sets the number.

    Sets the number to \a number, and expects nothing more.
*/

} // namespace QDocTests
