// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/*!
    \class StronglyOrderedClass
    \inmodule TestQDoc
    \compares strong
*/

/*!
    \class PartiallyOrderedClass
    \inmodule TestQDoc
    \compares partial
*/

/*!
    \class WeaklyOrderedClass
    \inmodule TestQDoc
    \compares weak
*/

/*!
    \class EqualityComparableClass
    \inmodule TestQDoc
    \compares equality
*/

/*!
    \class ComparesStronglyWithTwoClasses
    \inmodule TestQDoc

    \compareswith strong Foo
    \endcompareswith

    \compareswith strong Bar
    \warning Always compare twice!
    \endcompareswith
*/

/*!
    \class ComparesStronglyWithThreeClasses
    \inmodule TestQDoc

    \compareswith strong Foo {Bar Bar Jinks} Baz
    \endcompareswith
*/

/*!
    \class ComparesStronglyWithThreeClassesAcrossMultipleLines
    \inmodule TestQDoc

    \compareswith strong Foo \
                         Bar Baz
    \endcompareswith
*/

/*!
    \class ComparesStronglyWithOneClassAndPartiallyWithAnother
    \inmodule TestQDoc
    \brief Class with various comparison relationships.

    \compareswith strong Foo
    \endcompareswith

    \compareswith partial Bar
    Here we describe partial comparison with Bar.
    \endcompareswith

    //! using a macro
    \equalitycomparesto {Foo}

    This class compares strongly with one type, and partially with another.
*/


/*!
    \class Foo
    \inmodule TestQDoc
*/

/*!
    \class Bar
    \inmodule TestQDoc
*/

/*!
    \class Baz
    \inmodule TestQDoc
*/
