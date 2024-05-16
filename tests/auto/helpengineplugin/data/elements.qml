// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQml

QtObject {

    Component {
        id: comp
        QtObject {
            property url a: comp.url
        }
    }

    Timer {
         interval: 500; running: true; repeat: true
         onTriggered: restart()
     }
}
