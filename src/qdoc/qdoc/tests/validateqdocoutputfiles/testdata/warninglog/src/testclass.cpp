// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testclass.h"

/*!
    \module RelativePathsTest
    \title Relative Paths Test Module
    \brief Test module for relative path functionality.
*/
/*!
    \class TestClass
    \inmodule RelativePathsTest
    \brief A test class to verify relative path functionality in warning logs.
*/

/*!
    \brief Constructor with missing parameter documentation

    This function has an undocumented parameter to trigger warnings.
*/
void TestClass::partiallyDocumented(int param)
{
    // Implementation
}

/*!
    A documented function.
*/
void TestClass::documentedFunction()
{
    // Implementation
}

void TestClass::undocumentedFunction()
{
    // Implementation - this will generate a warning
}

