// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

#include "classwithoverloads.h"

/*
    \module TestModule

    Just a test module.
 */

/*!
    \class TestClass
    \inmodule TestModule

    \brief A class for the purpose of testing QDoc's \c {\overload} command.
 */

/*!
    \overload

    A member function without an overload that claims to be an overload by using
    the \c {\overload} command.
 */
void TestClass::method() {}

/*!
    A function that is overload unaware.
 */
void TestClass::another_method() {}

/*!
    \overload TestClass::another_method()

    A function that overloads another and correctly documents it.
 */
void TestClass::another_method(int number) {}

/*!
    \overload

    A function that overloads another and documents it vaguely.
 */
void TestClass::another_method(float overloads) {}
