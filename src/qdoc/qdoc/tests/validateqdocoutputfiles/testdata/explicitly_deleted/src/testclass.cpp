// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testclass.h"

/*!
    \module QDocTests
    \brief The QDocTests module contains test classes for QDoc.
*/

/*!
    \namespace QDocTests
    \inmodule QDocTests
    \brief The QDocTests namespace contains test classes for QDoc.
*/

/*!
    \class QDocTests::ExplicitlyDeletedTest
    \inmodule QDocTests
    \brief A test class for explicitly deleted special member functions.

    This class tests that QDoc correctly detects and displays
    functions marked with \c{= delete}.
*/

/*!
    \fn QDocTests::ExplicitlyDeletedTest::ExplicitlyDeletedTest()

    Default constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDeletedTest::ExplicitlyDeletedTest(const ExplicitlyDeletedTest &other)

    Copy constructor (deleted).
*/

/*!
    \fn QDocTests::ExplicitlyDeletedTest::ExplicitlyDeletedTest(ExplicitlyDeletedTest &&other)

    Move constructor (deleted).
*/

/*!
    \fn ExplicitlyDeletedTest &QDocTests::ExplicitlyDeletedTest::operator=(const ExplicitlyDeletedTest &other)

    Copy assigns \a other to this object (deleted).
*/

/*!
    \fn ExplicitlyDeletedTest &QDocTests::ExplicitlyDeletedTest::operator=(ExplicitlyDeletedTest &&other)

    Move assigns \a other to this object (deleted).
*/

