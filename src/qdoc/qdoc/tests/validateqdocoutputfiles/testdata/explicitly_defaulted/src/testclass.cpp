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
    \class QDocTests::ExplicitlyDefaultedTest
    \inmodule QDocTests
    \brief A test class for explicitly defaulted special member functions.

    This class tests that QDoc correctly detects and displays
    functions marked with \c{= default}.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::ExplicitlyDefaultedTest()

    Default constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::ExplicitlyDefaultedTest(const ExplicitlyDefaultedTest &other)

    Copy constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::ExplicitlyDefaultedTest(ExplicitlyDefaultedTest &&other)

    Move constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::~ExplicitlyDefaultedTest()

    Destructor.
*/

/*!
    \fn ExplicitlyDefaultedTest &QDocTests::ExplicitlyDefaultedTest::operator=(const ExplicitlyDefaultedTest &other)

    Copy assigns \a other to this object.
*/

/*!
    \fn ExplicitlyDefaultedTest &QDocTests::ExplicitlyDefaultedTest::operator=(ExplicitlyDefaultedTest &&other)

    Move assigns \a other to this object.
*/

