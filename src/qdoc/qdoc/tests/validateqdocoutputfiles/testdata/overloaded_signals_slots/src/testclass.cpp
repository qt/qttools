// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testclass.h"

/*!
    \class TestClass
    \inmodule OverloadedSignalsSlots
    \brief Test class for overloaded signals and slots.
*/

/*!
    \module OverloadedSignalsSlots
    \title Overloaded Signals Slots Test Module
    \brief Test module for overloaded signals and slots.
*/

/*!
    \fn TestClass::TestClass()

    Constructs a TestClass.
*/

/*!
    \fn void TestClass::regularFunction(int value)

    Regular function that takes an integer \a value.
*/

/*!
    \fn void TestClass::regularFunction(const String &value)
    \overload

    Regular function that takes a string \a value.
*/

/*!
    \fn void TestClass::dataChanged(int value)

    This method takes an integer \a value.
    (Tests overload note for methods that aren't signals/slots)
*/

/*!
    \fn void TestClass::dataChanged(const String &value)
    \overload

    This method takes a string \a value.
*/

/*!
    \fn void TestClass::process(int value)

    Processes an integer \a value.
    (Tests overload note for methods that aren't signals/slots)
*/

/*!
    \fn void TestClass::process(const String &value)
    \overload

    Processes a string \a value.
*/

