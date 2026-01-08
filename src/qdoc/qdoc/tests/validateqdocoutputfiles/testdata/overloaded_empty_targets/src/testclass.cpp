// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testclass.h"

/*!
    \class TestClass
    \inmodule OverloadedEmptyTargets
    \brief Test class for empty link target configuration.

    This class tests that QDoc handles empty overloadedsignalstarget
    and overloadedslotstarget configuration values without crashing.
*/

/*!
    \module OverloadedEmptyTargets
    \title Overloaded Empty Targets Test Module
    \brief Test module for empty link target configuration.
*/

/*!
    \fn TestClass::TestClass(QObject *parent)
    Constructs a TestClass with the given \a parent.
*/

/*!
    \fn void TestClass::valueChanged(int value)
    Signal emitted when an integer \a value changes.
*/

/*!
    \fn void TestClass::valueChanged(double value)
    Signal emitted when a double \a value changes.
*/

/*!
    \fn void TestClass::setValue(int value)
    Sets the integer \a value.
*/

/*!
    \fn void TestClass::setValue(double value)
    Sets the double \a value.
*/

