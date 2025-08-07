// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

/*!
    \qmltype BaseType
    \inqmlmodule TestModule
    \brief A basic QML type for testing imports.
*/

Item {
    id: root

    /*!
        \qmlproperty string BaseType::title
        The title of the item.
    */
    property string title: "Base"

    /*!
        \qmlmethod void BaseType::doSomething()
        Does something useful.
    */
    function doSomething() {
        console.log("BaseType doing something")
    }
}
