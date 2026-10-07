// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/*!
    \module CaseDistinctAnchors
    \title Case Distinct Anchors
*/

/*!
    \class Foo
    \inmodule CaseDistinctAnchors
    A class with a constructor and a related function.
*/
class Foo
{
public:
    /*! Constructs a Foo. */
    Foo();

    /*! \deprecated This member is obsolete. Use \l foo() instead. */
    void old();
};

/*!
    Computes a value for \a value.
    \relates Foo
*/
int foo(const Foo &value);

/*!
    \class Bar
    \inmodule CaseDistinctAnchors
    A class whose obsolete member does not link to the related function.
*/
class Bar
{
public:
    /*! Constructs a Bar. */
    Bar();

    /*! \deprecated This member is obsolete. */
    void old();
};

/*!
    Computes a value for \a value.
    \relates Bar
*/
int bar(const Bar &value);
