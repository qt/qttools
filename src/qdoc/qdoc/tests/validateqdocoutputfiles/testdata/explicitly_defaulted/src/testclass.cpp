// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testclass.h"

/*!
    \module QDocTests
    \brief The QDocTests module contains test classes for QDoc.
*/

/*!
    \namespace QDocTests
    \inmodule QDocTests
    \brief The QDocTests namespace contains test classes for QDoc.
*/

/*!
    \class QDocTests::ExplicitlyDefaultedTest
    \inmodule QDocTests
    \brief A test class for explicitly defaulted special member functions.

    This class tests that QDoc correctly detects and displays
    functions marked with \c{= default}.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::ExplicitlyDefaultedTest()

    Default constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::ExplicitlyDefaultedTest(const ExplicitlyDefaultedTest &other)

    Copy constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::ExplicitlyDefaultedTest(ExplicitlyDefaultedTest &&other)

    Move constructor.
*/

/*!
    \fn QDocTests::ExplicitlyDefaultedTest::~ExplicitlyDefaultedTest()

    Destructor.
*/

/*!
    \fn ExplicitlyDefaultedTest &QDocTests::ExplicitlyDefaultedTest::operator=(const ExplicitlyDefaultedTest &other)

    Copy assigns \a other to this object.
*/

/*!
    \fn ExplicitlyDefaultedTest &QDocTests::ExplicitlyDefaultedTest::operator=(ExplicitlyDefaultedTest &&other)

    Move assigns \a other to this object.
*/

/*!
    \class QDocTests::UnnamedParameterTest
    \inmodule QDocTests
    \brief A test class with unnamed parameters in assignment operators.

    This class tests that QDoc doesn't emit spurious "No such parameter"
    warnings for auto-generated documentation when the parameter is
    unnamed in the source code.
*/

/*!
    \class QDocTests::DeclaredConstexprTest
    \inmodule QDocTests
    \brief A test class for explicitly defaulted functions declared constexpr.

    This class tests that QDoc marks an explicitly defaulted function as
    \c constexpr only when its declaration says so, either directly or
    through a macro.
*/

/*!
    \fn QDocTests::DeclaredConstexprTest::DeclaredConstexprTest()

    Default constructor, declared \c constexpr.
*/

/*!
    \fn QDocTests::DeclaredConstexprTest::DeclaredConstexprTest(const DeclaredConstexprTest &other)

    Copy constructor, declared \c constexpr through a macro.
*/

/*!
    \fn QDocTests::DeclaredConstexprTest::DeclaredConstexprTest(DeclaredConstexprTest &&other)

    Move constructor, not declared \c constexpr.
*/

/*!
    \class QDocTests::MacroDeclaredTest
    \inmodule QDocTests
    \brief A test class with explicitly defaulted functions declared by a macro.

    This class tests that QDoc doesn't mark an explicitly defaulted
    function as \c constexpr when the macro that declares it doesn't say so.
*/

/*!
    \fn QDocTests::MacroDeclaredTest::MacroDeclaredTest()

    Default constructor, not declared \c constexpr.
*/

/*!
    \fn QDocTests::MacroDeclaredTest::MacroDeclaredTest(const MacroDeclaredTest &other)

    Copy constructor, declared by a macro without \c constexpr.
*/

/*!
    \fn QDocTests::MacroDeclaredTest &QDocTests::MacroDeclaredTest::operator=(const MacroDeclaredTest &other)

    Copy assigns \a other to this object. Declared by a macro without
    \c constexpr.
*/

/*!
    \class QDocTests::MacroDeclaredConstexprTest
    \inmodule QDocTests
    \brief A test class with explicitly defaulted functions declared constexpr by a macro.

    This class tests that QDoc marks an explicitly defaulted function as
    \c constexpr when the macro that declares it says so.
*/

/*!
    \fn QDocTests::MacroDeclaredConstexprTest::MacroDeclaredConstexprTest()

    Default constructor, declared \c constexpr.
*/

/*!
    \fn QDocTests::MacroDeclaredConstexprTest::MacroDeclaredConstexprTest(const MacroDeclaredConstexprTest &other)

    Copy constructor, declared \c constexpr by a macro.
*/

/*!
    \fn QDocTests::MacroDeclaredConstexprTest &QDocTests::MacroDeclaredConstexprTest::operator=(const MacroDeclaredConstexprTest &other)

    Copy assigns \a other to this object. Declared \c constexpr by a macro.
*/

/*!
    \class QDocTests::MacroPrefixTest
    \inmodule QDocTests
    \brief A test class with explicitly defaulted functions that start with a macro.

    This class tests that QDoc reads the definition of a macro that starts
    the declaration of an explicitly defaulted function, and marks the
    function as \c constexpr only when the macro says so.
*/

/*!
    \fn QDocTests::MacroPrefixTest::MacroPrefixTest()

    Default constructor, starting with a macro that doesn't declare it
    \c constexpr.
*/

/*!
    \fn QDocTests::MacroPrefixTest::MacroPrefixTest(const MacroPrefixTest &other)

    Copy constructor, starting with a macro that expands to another macro.
    Neither declares it \c constexpr.
*/

/*!
    \fn QDocTests::MacroPrefixTest::MacroPrefixTest(MacroPrefixTest &&other)

    Move constructor, starting with a macro that declares it \c constexpr
    on its second line.
*/

/*!
    \class QDocTests::DeletedCopyCtorTest
    \inmodule QDocTests
    \brief A test class with a deleted copy constructor.

    This class tests that QDoc correctly resolves links to constructors
    when a deleted copy constructor is present. The deleted copy constructor
    should not be chosen as the link target.
*/

/*!
    \fn QDocTests::DeletedCopyCtorTest::DeletedCopyCtorTest(int value, const char *name, void *data)

    Constructs a DeletedCopyCtorTest with the given \a value, \a name, and \a data.
*/

/*!
    \fn void QDocTests::DeletedCopyCtorTest::someMethod()

    Does something.

    \sa DeletedCopyCtorTest()
*/

