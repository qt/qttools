/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "testcpp.h"

namespace TestQDoc {

/*!
    \module TestCPP
    \qtvariable testcpp
    \title QDoc Test C++ Classes
    \brief A test module page.
*/

/*!
    \namespace TestQDoc
    \inheaderfile TestCPP
    \inmodule TestCPP
    \brief A namespace.

    \section1 Usage
    This namespace is for testing QDoc output.
*/

/*!
    \class TestQDoc::Test
    \inmodule TestCPP
    \brief A class in a namespace.
*/

/*!
    \class TestQDoc::TestDerived
    \inmodule TestCPP
    \brief A derived class in a namespace.
*/

/*!
    \macro QDOCTEST_MACRO
    \relates TestQDoc
*/

/*!
    \macro QDOCTEST_MACRO2(x)
    \relates TestQDoc::Test
    \since Test 1.1
    \brief A macro with argument \a x.
*/

/*!
    \deprecated

    Use someFunction() instead.
*/
void Test::deprecatedMember()
{
    return;
}

/*!
    \obsolete

    Use someFunction() instead.
*/
void Test::obsoleteMember()
{
    return;
}

/*!
    \obsolete

    Use obsoleteMember() instead.
*/
void Test::anotherObsoleteMember()
{
    return;
}

/*!
    Function that takes a parameter \a i and \a b.
*/
void Test::someFunctionDefaultArg(int i, bool b = false)
{
    return;
}

/*!
    Function that takes a parameter \a v.
    Also returns the value of \a v.
*/
int Test::someFunction(int v)
{
    return v;
}

/*!
    \fn void TestQDoc::Test::inlineFunction()

    \brief An inline function, documented using the \CMDFN QDoc command.
*/

/*!
    Function that must be reimplemented.
*/
void Test::virtualFun()
{
    return;
}

/*!
    \reimp
*/
void TestDerived::virtualFun()
{
    return;
}

/*!
    \fn TestQDoc::Test::overload()
    \fn TestQDoc::Test::overload(bool b)

    Overloads that share a documentation comment, optionally taking
    a parameter \a b.
*/

} // namespace TestQDoc
