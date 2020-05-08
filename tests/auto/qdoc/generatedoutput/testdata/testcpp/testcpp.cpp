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

    \testnoautolist

\if defined(test_nestedmacro)
    \versionnote {module} {\ver}
\endif
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

\if defined(test_ignoresince)
    //! omitted by ignoresince
    \since 1.1
\endif
*/

/*!
    \class TestQDoc::TestDerived
    \inmodule TestCPP
    \brief A derived class in a namespace.
*/

/*!
    \macro QDOCTEST_MACRO
    \relates TestQDoc
\if defined(test_ignoresince)
    //! omitted by ignoresince.Test
    \since Test 0.9
\endif
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
    \obsolete Use obsoleteMember() instead.
*/
void Test::anotherObsoleteMember()
{
    return;
}

/*!
    Function that takes a parameter \a i and \a b.
\if defined(test_ignoresince)
    \since 2.0
\endif
*/
void Test::someFunctionDefaultArg(int i, bool b = false)
{
    return;
}

// Documented below with an \fn command. Unnecessary but we support it, and it's used.
int Test::someFunction(int v)
{
    return v;
}

/*!
    \fn void TestQDoc::Test::inlineFunction()

    \brief An inline function, documented using the \CMDFN QDoc command.
*/

/*!
    \fn int Test::someFunction(int v = 0)

    Function that takes a parameter \a v.
    Also returns the value of \a v.
\if defined(test_ignoresince)
    \since Test 1.0
\endif
*/

/*!
    Function that must be reimplemented.
*/
void Test::virtualFun()
{
    return;
}

/*!
    \typedef Test::SomeType
    \brief A typedef.
*/

/*!
    \reimp
*/
void TestDerived::virtualFun()
{
    return;
}

/*!
    \fn TestQDoc::Test::overload()
    \fn Test::overload(bool b)
    \since Test 1.2
     //! The second overload should match even without the fully qualified path

    Overloads that share a documentation comment, optionally taking
    a parameter \a b.
*/

/*!
    \typealias TestDerived::DerivedType
    An aliased typedef.
*/

/*!
    \typedef TestDerived::NotTypedef
    I'm an alias, not a typedef.
*/

/*!
\if defined(test_template)
    \fn template <typename T1, typename T2> void TestQDoc::Test::funcTemplate(T1 a, T2 b)
    \brief Function template with two parameters, \a a and \a b.
\else
    \nothing
\endif
*/

/*!
\if defined(test_template)
    \struct TestQDoc::Test::Struct
    \inmodule TestCPP
    \brief Templated struct.
\else
    \nothing
\endif
*/

/*!
\if defined(test_template)
    \typealias TestQDoc::Test::Specialized
\else
    \nothing
\endif
*/

} // namespace TestQDoc
