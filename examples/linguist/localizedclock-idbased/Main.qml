// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import qtexamples.localizedclock

ApplicationWindow {
    id: root
    width: 320
    height: 320
    visible: true
    minimumWidth: layout.Layout.minimumWidth
    minimumHeight: layout.Layout.minimumHeight
    //% "Digital Clock"
    title: qsTrId("Main-Digital-Clock")
    property string time
    property string date
    property int seconds
    property int diff

    ColumnLayout {
        anchors.fill: parent
        id: layout
        Label {
            text: root.time
            fontSizeMode: Text.Fit
            font.pixelSize: 60
            minimumPixelSize: 10
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            //% "%n second(s)"
            text: qsTrId("Main-n-second-s", root.seconds)
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            text: root.date
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }
        Label {
            //% "Time zone: "
            text: qsTrId("timezone") + TimeZoneManager.timeZone;
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }
        Button {
            text: qsTrId("timezonelabel")
            onClicked: TimeZoneManager.openDialog()
            Layout.alignment: Qt.AlignHCenter
        }
    }
    Connections {
        target: TimeZoneManager
        function onTimeZoneChanged() {
            root.diff = TimeZoneManager.currentTimeZoneOffsetMs();
        }
    }
    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true

        onTriggered: {
            const now = new Date(new Date().getTime() + root.diff);
            const locale = Qt.locale();
            root.time = now.toLocaleTimeString(locale, Locale.ShortFormat);
            root.date = now.toLocaleDateString(locale);
            root.seconds = now.getSeconds();
        }
    }
}
