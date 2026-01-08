// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testclass.h"

/*!
    \class TestClass
    \inmodule OverloadedSignalsSlots
    \brief Test class for overloaded signals and slots.

    This class tests QDoc's generation of connection snippets for
    overloaded signals and slots.
*/

/*!
    \module OverloadedSignalsSlots
    \title Overloaded Signals Slots Test Module
    \brief Test module for overloaded signals and slots.
*/

/*!
    \fn TestClass::TestClass(QObject *parent)

    Constructs a TestClass with the given \a parent.
*/

/*!
    \fn void TestClass::dataChanged(int value)

    This signal is emitted when an integer \a value changes.
*/

/*!
    \fn void TestClass::dataChanged(const QString &value)

    This signal is emitted when a string \a value changes.
*/

/*!
    \fn void TestClass::process(int value)

    Processes an integer \a value.
*/

/*!
    \fn void TestClass::process(const QString &value)

    Processes a string \a value.
*/
