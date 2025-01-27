// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick 2.0

/*!
    \qmltype DocTest
    \inqmlmodule QDoc.Test
    \brief Represents a doc test case.
    \since QDoc.Test 0.9

    \section1 Introduction

    A documentation test case, itself documented inline in \DocTest.qml.
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
    Signals that something is \a really happening.
    */
    signal itsHappening(bool really)

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
    required property string name

    /*!
        Magic numbers.
    */
    property list<int> numbers


    /*!
        \qmlproperty list<Item> items
        List of items.
    */
    property list<Item> items

    /*!
        \qmlproperty list<bool> boolNumbers
        Property with overridden type.
    */
    property list<int> boolNumbers

    /*!
        Whether the test is active.
        \default true

        \sa name
    */
    property bool active: true

    /*! \internal */
    property int doctest_internal: -1

    /*!
    \qmlproperty string icon.source
    \qmlproperty string icon.name

    Holds an icon.
    */
    property Icon icon
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
