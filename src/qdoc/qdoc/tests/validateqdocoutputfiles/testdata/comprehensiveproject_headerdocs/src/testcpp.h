// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

#include <QtCore/qmetaobject.h>
#include <QtCore/qproperty.h>
#include <QtCore/qstring.h>

/*!
    \macro QDOCTEST_MACRO
    \relates TestQDoc
    \since Test 0.9
*/
#define QDOCTEST_MACRO test

/*!
    \macro QDOCTEST_MACRO2(int &x)
    \relates TestQDoc::Test
    \since Test 1.1
    \brief A macro with argument \a x.
    \ingroup testgroup
*/
#define QDOCTEST_MACRO2(x) (x) < 0 ? 0 : (x)

/*!
    \namespace TestQDoc
    \inheaderfile TestCPP
    \inmodule TestCPP
    \brief A namespace.

    \section1 Usage
    This namespace is for testing QDoc output.
*/
namespace TestQDoc {
/*!
    \class TestQDoc::Test
    \inmodule TestCPP
    \brief A class in a namespace.

    \since 1.1
    \ingroup testgroup
    \ingroup cpptypes
    \reentrant
*/
class Test {
    Q_OBJECT
    Q_PROPERTY(int id READ id)
public:

/*!
    \struct TestQDoc::Test::Struct
    \inmodule TestCPP
    \brief Templated struct.
*/

/*!
    \typealias TestQDoc::Test::Specialized
*/
template<typename D, typename T> struct Struct {};
template<typename T>
using Specialized = Struct<int, T>;

/*!
    \typedef TestQDoc::Test::SomeType
    \brief A typedef.
*/
    typedef struct {
        int data;
    } SomeType;

// Documented below with an \fn command. Unnecessary but we support it, and it's used.
    int someFunction(int, int v = 0);

/*!
    \fn void TestQDoc::Test::someFunctionDefaultArg(int i, bool b = false) const
    \nonreentrant
    Function that takes a parameter \a i and \a b.

    \since 2.0
    \ingroup testgroup
*/
    void someFunctionDefaultArg(int i, bool b) const;

/*!
    \fn int Test::someFunction(int, int v = 0)

    Function that takes a parameter \a v.
    Also returns the value of \a v.

    \since Test 1.0
*/

/*!
    \fn void TestQDoc::Test::obsoleteMember()
    \obsolete

    Use someFunction() instead.
*/
    void obsoleteMember();

/*!
    \fn void TestQDoc::Test::anotherObsoleteMember()
    \obsolete Use obsoleteMember() instead.
*/
    void anotherObsoleteMember();

/*!
    \fn TestQDoc::Test::deprecatedMember()
    \deprecated [6.0] Use someFunction() instead.
*/
    void deprecatedMember();

/*!
    \fn void TestQDoc::Test::methodWithEnDashInItsDocs();

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
    void methodWithEnDashInItsDocs();

/*!
    \fn TestQDoc::Test::methodWithEmDashInItsDocs()
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
    void methodWithEmDashInItsDocs();

/*!
    \fn void Test::func(bool)
    \internal
*/
    void func(bool) {};

    //! [funcPtr]
/*!
    \fn [funcPtr] void (*funcPtr(bool b, const char *s))(bool)

    Returns a pointer to a function that takes a boolean. Uses \a b and \a s.
*/
    void (*funcPtr(bool b, const char *s))(bool) {
        return func;
    }

/*!
    \fn Test& Test::operator++()
    \fn Test& Test::operator--()
    \deprecated
*/
    Test &operator++() { return *this; }
    Test &operator--() { return *this; }

    void anotherFunc() {};

/*!
    \fn void TestQDoc::Test::inlineFunction()

    \brief An inline function, documented using the \CMDFN QDoc command.
*/
    inline void inlineFunction() {};

/*!
    \fn void TestQDoc::Test::virtualFun()

    Function that must be reimplemented.
*/
    virtual void virtualFun();

/*!
    //! \fn bool Test::operator==(const Test &lhs, const Test &rhs)

    Returns true if \a lhs and \a rhs are equal.
*/
    friend bool operator==(const Test &lhs, const Test &rhs) { return false; }

/*!
    \fn TestQDoc::Test::Test()

    The constructor is deleted.
*/
    Test() = delete;

/*!
    \fn Test &Test::operator=(Test &&other)
    \ingroup testgroup

    The move assignment operator is deleted. \a other cannot be moved from.
*/
    Test &operator=(Test &&other) = delete;

protected:
/*!
    \fn TestQDoc::Test::overload()
    \fn Test::overload(bool b)
     //! The second overload should match even without the fully qualified path

    Overloads that share a documentation comment, optionally taking
    a parameter \a b.
*/
    void overload() {}
/*!
    \fn Test::overload(bool b)
    \since Test 1.2
*/

    void overload(bool b) { if (!b) return; }
/*!
    \brief Function template with two parameters, \a a and \a b.
*/
    template <typename T1, typename T2> void funcTemplate(T1 a, T2 b) {
        a = b;
    }
/*!
    \property TestQDoc::Test::id
*/
    virtual int id() { return 0; }
};

/*!
    \class TestQDoc::TestDerived
    \inmodule TestCPP
    \brief A class in a namespace, derived from \l [CPP] Test.
*/
class TestDerived : public Test {
    Q_OBJECT

    Q_PROPERTY(QString bindableProp READ bindableProp WRITE setBindableProp NOTIFY bindablePropChanged BINDABLE bindableProp)
    Q_PROPERTY(QString someProp READ someProp BINDABLE somBindableProp)
    Q_PROPERTY(int *intProp READ getInt STORED false CONSTANT FINAL)
    Q_PROPERTY(const QString *name READ name)
    QDOC_PROPERTY(bool boolProp READ boolProp WRITE setBoolProp NOTIFY boolPropChanged RESET resetBoolProp REVISION 1)
    QDOC_PROPERTY(bool secondBoolProp READ secondBoolProp NOTIFY boolPropChanged)

public:

    /*!
        \value Val1
        \value Val2
    */
    enum { Val1, Val2 };

    /*!
        \value Val3
        \value Val4
    */
    enum { Val3, Val4 };
/*!
    \typealias TestQDoc::TestDerived::DerivedType
    An aliased typedef.
*/
    using DerivedType = Test::SomeType;

/*!
    \typedef TestQDoc::TestDerived::NotTypedef
    I'm an alias, not a typedef.
*/
    using NotTypedef = int;
/*!
    \fn void TestQDoc::TestDerived::virtualFun()
    \reimp
*/
    void virtualFun() override;

/*!
    \fn static void TestQDoc::TestDerived::staticObsoleteMember()
    \obsolete

    Static obsolete method.
*/
    static void staticObsoleteMember();

/*!
    \fn NotTypedef TestQDoc::TestDerived::someValue()
    Returns a value using an aliases type.
*/
    NotTypedef someValue();
    QBindable<QString> bindableProp();
    QBindable<QString> someBindableProp();
    const QString &someProp();
    int *getInt();
    bool boolProp();
    bool secondBoolProp() { return boolProp(); }
    const QString *name() const;

/*!
    \fn TestQDoc::TestDerived::invokeMe() const
    \brief Something invokable.
*/
    Q_INVOKABLE void invokeMe() const {}

/*!
    \fn int TestQDoc::TestDerived::id()
    \reimp

*/
    int id() override;

Q_SIGNALS:
/*!
    \property TestQDoc::TestDerived::bindableProp
    Some property.

    \sa someProp
*/

/*!
    \property TestQDoc::TestDerived::someProp
    Another property.
*/

/*!
    \property TestQDoc::TestDerived::name
    \brief a name.
*/

/*!
    \property TestQDoc::TestDerived::intProp
    An integer property.
*/

/*!
    \property TestQDoc::TestDerived::boolProp
    A boolean property.
*/

/*!
    \property TestQDoc::TestDerived::secondBoolProp
    A property sharing a notifier signal with boolProp.
*/

/*!
   Emitted when things happen.
*/
    void emitSomething(QPrivateSignal);
    void bindablePropChanged();
    Q_REVISION(1) void boolPropChanged();

public Q_SLOTS:
    void setBindableProp(const QString &s);
    void setBoolProp(bool b);
    void resetBoolProp();
};

/*!
    \class TestQDoc::Vec
    \inmodule TestCPP
    \brief Type alias that has its own reference.
*/
template <typename T>
struct BaseVec {};
template <typename T>
using Vec = BaseVec<T>;

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
    void documentMe() {};
} // namespace CrossModuleRef

/*!
    \class DontLinkToMe
    \inmodule TestCPP
    \brief Class that does not generate documentation.
*/
class DontLinkToMe {};
/*!
    \dontdocument (DontLinkToMe)
*/

