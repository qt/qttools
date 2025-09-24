// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "testcpp.h"

/*!
    \macro QDOCTEST_CONDITIONAL_MACRO
    \relates TestQDoc
    \since Test 1.3

    Test conditional macro documented separately from its definition.
    This replicates the Qt pattern where macros are defined in headers
    but documented in cpp files.
*/

/*!
    \macro QDOCTEST_SHARED_MACRO_A(x)
    \macro QDOCTEST_SHARED_MACRO_B(x)
    \relates TestQDoc
    \since Test 1.4

    These macros perform simple arithmetic operations on \a x.
    QDOCTEST_SHARED_MACRO_A increments the value by 1.
    QDOCTEST_SHARED_MACRO_B decrements the value by 1.

    These macros share documentation to test plural form in \c {\since} text.
*/

namespace TestQDoc {

/*
//! [random tag]
\note This is just a test.
//! [random tag]

//! [args]
\1\2 \3 \2\1
//! [args]
*/

void Test::deprecatedMember()
{
    return;
}

void Test::obsoleteMember()
{
    return;
}

void Test::anotherObsoleteMember()
{
    return;
}

void Test::someFunctionDefaultArg(int i, bool b = false) const
{
    return;
}

void Test::methodWithEnDashInItsDocs()
{
    // Nothing to see here.
}

void Test::methodWithEmDashInItsDocs()
{
    // Woah! Look at that!
}

int Test::someFunction(int, int v)
{
    return v;
}

void Test::virtualFun()
{
    return;
}

void TestDerived::virtualFun()
{
    return;
}

void TestDerived::staticObsoleteMember()
{
    return;
}

int TestDerived::id()
{
    return 1;
}

TestDerived::NotTypedef TestDerived::someValue()
{
    return 0;
}

} // namespace TestQDoc

