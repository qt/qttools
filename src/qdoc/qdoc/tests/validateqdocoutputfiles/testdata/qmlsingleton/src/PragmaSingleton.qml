// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

pragma singleton

import QtQuick 2.0

/*!
    \qmlmodule QmlSingletonTest 1.0
    \brief Test module for QML singleton documentation.
*/

/*!
    \qmltype PragmaSingleton
    \inqmlmodule QmlSingletonTest
    \since QmlSingletonTest 1.0
    \brief A QML singleton type using pragma singleton.

    This type demonstrates automatic singleton detection using
    the QML pragma singleton directive.
*/
QtObject {
    id: root

    /*!
        \qmlproperty string PragmaSingleton::version
        \readonly

        The version of this singleton.
    */
    readonly property string version: "1.0"

    /*!
        \qmlmethod string PragmaSingleton::getInfo()

        Returns information about this singleton.
    */
    function getInfo() {
        return "PragmaSingleton v" + version
    }
}
