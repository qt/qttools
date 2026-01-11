// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testtemplate.h"

/*!
    \class Foo
    \inmodule TestCPP
    \brief Class template.

    \a T is the value type.
*/

/*!
    \class Bar
    \inmodule TestCPP
    \brief Another class template.

    \a T is the first type, \a D is the second type.
*/

/*!
    //! Baz is a struct, QDoc auto-converts this to the correct type
    \class Baz
    \inmodule TestCPP
    \brief Class template template.

    \a X is the template-template parameter, \a Y is the type used with it.
*/

/*!
    \class Outer
    \inmodule TestCPP
    \brief Outer class template for testing nested template parameter visibility.

    \a T is the outer element type.
*/

/*!
    \class Outer::Inner
    \inmodule TestCPP
    \brief Nested class template inside Outer.

    Inner can reference outer type \a T without requiring redundant documentation.

    \a U is the inner element type.
*/
