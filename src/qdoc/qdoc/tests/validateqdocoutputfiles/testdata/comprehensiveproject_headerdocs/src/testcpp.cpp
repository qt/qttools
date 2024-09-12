// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "testcpp.h"

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

