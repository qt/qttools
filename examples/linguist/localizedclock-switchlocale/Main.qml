// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import qtexamples.localizedclockswitchlocale

pragma ComponentBehavior: Bound

Window {
    id: root
    width: 320
    height: 320
    visible: true
    minimumWidth: layout.Layout.minimumWidth
    minimumHeight: layout.Layout.minimumHeight

    function updateClock() {
        const now = new Date()
        const locale = Qt.locale()
        root.time    = now.toLocaleTimeString(locale, Locale.ShortFormat)
        root.date    = now.toLocaleDateString(locale)
        root.seconds = now.getSeconds()
    }

    Component.onCompleted: {
        TranslatorManager.switchLanguage("en")
        updateClock()
    }

    title: qsTr("Digital Clock")
    property string time
    property string date
    property int seconds
    property string selectedLanguage: "English"

    Button {
        id: languageButton
        anchors.top: parent.top
        anchors.right: parent.right
        width: 120
        height: 40
        anchors.margins: 5
        background: Rectangle {
            color: "transparent"
        }
        RowLayout {
            anchors.centerIn: parent
            Image {
                source: "globe.png"
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
            }

            Text {
                text: root.selectedLanguage
                color: "gray"
                font.pixelSize: 18
                verticalAlignment: Text.AlignVCenter
            }
        }
        ListModel {
            id: languageModel
            ListElement { name: "English"; code: "en" }
            ListElement { name: "Deutsch"; code: "de" }
            ListElement { name: "العربية"; code: "ar" }
            ListElement { name: "한국어"; code: "ko" }
            ListElement { name: "中文"; code: "zh" }
            ListElement { name: "日本語"; code: "ja" }
            ListElement { name: "Français"; code: "fr" }
            ListElement { name: "Italiano"; code: "it" }
            ListElement { name: "Español"; code: "es" }
            ListElement { name: "Português"; code: "pt" }
        }
        Menu {
            id: languageMenu
            width: languageButton.width
            Repeater {
                model: languageModel
                delegate: MenuItem {
                    required property string name
                    required property string code
                    text: name
                    onTriggered: {
                        TranslatorManager.switchLanguage(code)
                        root.selectedLanguage = name
                        root.updateClock()
                    }
                }
            }
        }
        onClicked: languageMenu.open()
    }
    ColumnLayout {
        anchors.fill: parent
        id: layout
        Text {
            text: root.time
            fontSizeMode: Text.Fit
            font.pixelSize: 60
            minimumPixelSize: 10
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: qsTr("%n second(s)", "seconds", root.seconds)
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }
        Text {
            text: root.date
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }
    }
    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true

        onTriggered: root.updateClock()
    }
}
