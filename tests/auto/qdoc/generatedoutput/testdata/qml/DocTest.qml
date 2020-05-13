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

import QtQuick 2.0

/*!
    \qmltype DocTest
    \inherits Test
    \inqmlmodule QDoc.Test
    \brief Represents a doc test case.
    \since QDoc.Test 0.9

    \section1 Introduction

    A documentation test case, itself documented inline in DocTest.qml.
*/
Item {
    id: testCase

    /*!
        \qmlsignal QDocTest::completed
    */
    signal completed

    /*!
        \qmlsignal DocTest::test(var bar)
        Signal with parameter \a bar.
    */
    signal foo(var bar)

    /*!
        \qmlproperty string DocTest::name

        Name of the test.
        \qml
        DocTest {
            name: "test"
            // ...
        }
        \endqml
    */
    property string name

    /*!
        Whether the test is active.

        \sa name
    */
    property bool active: true

    /*! \internal */
    property int doctest_internal: -1

    /*!
        \qmlmethod DocTest::fail(message = "oops")
        \since QDoc.Test 1.0

        Fails the current test case, with the optional \a message.
    */
    function fail(msg) {
        if (msg === undefined)
            msg = "oops";
    }

    /*! \internal */
    function doctest_fail(msg) {
        if (msg === undefined)
            msg = "";
    }

    /*!
    \brief Fails the current test case, hard.
    \list
        \li Prints out \a msg.
        \li Accepts a random \a option.
    \endlist
    */
    function fail_hard(msg = "facepalm", option = 123) {
    }
}
