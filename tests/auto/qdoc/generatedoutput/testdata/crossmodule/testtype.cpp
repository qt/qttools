// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "testtype.h"

/*!
    \module CrossModule
*/

/*!
    Function under a namespace that's documented elsewhere.
*/
void CrossModuleRef::documentMeToo()
{
}

/*!
    \class TestType
    \inmodule CrossModule
    \brief A class inheriting another class that lives in an external doc
           module.

    \section1 Linking

    These links go to the parent class:
    \list
      \li \l {TestQDoc::TestDerived}
      \li \l {TestQDoc::}{Test} class \l Usage.
      \li QDOCTEST_MACRO
      \li DontLinkToMe
    \endlist

    \section1 Generated Lists

    This is an annotated list of entries in a group:
    \annotatedlist testgroup

    \sa {TestQDoc::Test::}{someFunction()}
*/

/*!
    Nothing to see here.
*/
void TestType::nothing()
{
}
