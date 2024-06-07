// Copyright (C) 2019 The Qt Company Ltd.
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

/*!
    \module TestCPP
    \qtvariable testcpp
    \qtcmakepackage QDocTest
    \title QDoc Test C++ Classes
    \brief A test module page.
    \since 2.0

    \testnoautolist

    \include testcpp.cpp random tag
    \include testcpp.cpp {args} {/} {*} {Look, Ma! {I'm made of arguments!}}

\if defined(test_nestedmacro)
    \versionnote {module} {\ver 5.15.0}
    \ver 1.0.0
\endif

    \section1 Linking to function-like things

    \list
        \li \l {TestQDoc::Test::someFunctionDefaultArg}
               {someFunctionDefaultArg()}
        \li QDOCTEST_MACRO2()
        \li \l {TestQDoc::Test::}{QDOCTEST_MACRO2(int &x)}
        \li \l {section()}
        \li \l {section()} {section() is a section title}
    \endlist

    \section2 section()
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
    \ingroup testgroup
    \ingroup cpptypes
    \reentrant
*/

/*!
    \class TestQDoc::TestDerived
    \inmodule TestCPP
    \brief A class in a namespace, derived from \l [CPP] Test.
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
    \macro QDOCTEST_MACRO2(int &x)
    \relates TestQDoc::Test
    \since Test 1.1
    \brief A macro with argument \a x.
    \ingroup testgroup
*/

/*!
\if defined(test_properties)
    \property Test::id
\else
    \nothing
\endif
*/

/*!
    \deprecated [6.0] Use someFunction() instead.
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
    \nonreentrant
    Function that takes a parameter \a i and \a b.
\if defined(test_ignoresince)
    \since 2.0
\endif
    \ingroup testgroup
*/
void Test::someFunctionDefaultArg(int i, bool b = false) const
{
    return;
}

/*!
    \fn void Test::func(bool)
    \internal
*/

/*!
    \fn [funcPtr] void (*funcPtr(bool b, const char *s))(bool)

    Returns a pointer to a function that takes a boolean. Uses \a b and \a s.
*/

/*!
    \fn [op-inc] Test::operator++()
    \fn [op-dec] Test::operator--()
    \deprecated
*/

/*!
    This method has en dashes in its documentation -- as you'll find
    represented by \c{--} in the sources -- here and there. The important bit
    to note is that when passed e.g. to the \\c command, the two hyphens are
    processed as input to the command and not replaced by an en dash. This also
    applies to code blocks, where otherwise, the decrement operator would get
    completely borked:

    \code
    for (int i = 42; i > 0; --i)
        // Do something cool during countdown.
    \endcode

    ...as it would be silly if this would output --i instead of \c {--i}.

    -----------------------------------------------------------------------

    It still allows people to add a bunch of dashes, though, without replacing
    them all with a series of en dashes. Of course, they might want to use the
    \\hr command instead, like this:
    \hr

    -- You can also start a new paragraph with an en dash, if you want to.

    //! Self-referencing \sa-command for tests.
    \sa methodWithEnDashInItsDocs
*/
void Test::methodWithEnDashInItsDocs()
{
    // Nothing to see here.
}

/*!
    This method has em dashes in its documentation---as you'll find
    represented by \c{---} in the sources---here and there. The important bit
    to note is that when passed e.g. to the \\c command, the three hyphens are
    processed as input to the command and not replaced by an em dash.

    -----------------------------------------------------------------------

    People can still add a bunch of dashes, though, without QDoc replacing
    them all with a series of em dashes.

    ---You can also start a new paragraph with an em dash, if you want to.

    \sa methodWithEnDashInItsDocs

*/
void Test::methodWithEmDashInItsDocs()
{
    // Woah! Look at that!
}

// Documented below with an \fn command. Unnecessary but we support it, and it's used.
int Test::someFunction(int, int v)
{
    return v;
}

/*!
    \fn void TestQDoc::Test::inlineFunction()

    \brief An inline function, documented using the \CMDFN QDoc command.
*/

/*!
    \fn int Test::someFunction(int, int v = 0)

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
    \fn bool Test::operator==(const Test &lhs, const Test &rhs)

    Returns true if \a lhs and \a rhs are equal.
*/

/*!
    \typedef TestQDoc::Test::SomeType
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
     //! The second overload should match even without the fully qualified path

    Overloads that share a documentation comment, optionally taking
    a parameter \a b.
*/

/*!
    \fn Test::overload(bool b)
    \since Test 1.2
*/

/*!
    \typealias TestQDoc::TestDerived::DerivedType
    An aliased typedef.
*/

/*!
    \typedef TestQDoc::TestDerived::NotTypedef
    I'm an alias, not a typedef.
*/

/*!
    \obsolete

    Static obsolete method.
*/
void TestDerived::staticObsoleteMember()
{
    return;
}

/*!
\if defined(test_properties)
    \fn void TestDerived::emitSomething()
    Emitted when things happen.
\else
    \nothing
\endif
*/

/*!
\if defined(test_properties)
    \reimp
\else
    \nothing
\endif
*/
int TestDerived::id()
{
    return 1;
}

/*!
    Returns a value using an aliases type.
*/
TestDerived::NotTypedef TestDerived::someValue()
{
    return 0;
}

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

/*!
\if defined(test_template)
    \class TestQDoc::Vec
    \inmodule TestCPP
    \brief Type alias that has its own reference.
\else
    \nothing
\endif
*/

/*!
\if defined(test_template)
    \macro Q_INVOKABLE
    \relates TestQDoc::Test

    This is a mock Q_INVOKABLE for the purpose of ensuring QDoc autolink to it
    as expected.
\else
    \nothing
\endif
*/

} // namespace TestQDoc


/*!
    \namespace CrossModuleRef
    \inmodule TestCPP
    \brief Namespace that has documented functions in multiple modules.
    \since 3.0
*/
namespace CrossModuleRef {

/*!
    Document me!
*/
void documentMe()
{
}

} // namespace CrossModuleRef

/*!
    \class DontLinkToMe
    \inmodule TestCPP
    \brief Class that does not generate documentation.
*/

/*!
    \dontdocument (DontLinkToMe)
*/
