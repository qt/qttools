// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts

//! [filepicker]
Item {
    id: filePicker

    property string dialogText
    property alias selectedFile: filePathField.text

    height: addFileButton.height

    FileDialog {
        id: fileDialog
        title: qsTr("Please Choose %1").arg(filePicker.dialogText)
        currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
        fileMode: FileDialog.OpenFile
        onAccepted: filePathField.text = fileDialog.selectedFile
    }

    RowLayout {
        anchors.fill: parent
        TextField {
            id: filePathField
            placeholderText: qsTr("<%1>").arg(filePicker.dialogText)
            inputMethodHints: Qt.ImhUrlCharactersOnly
            selectByMouse: true
            Layout.fillWidth: true
        }

        Button {
            id: addFileButton
            text: qsTr("Add %1").arg(filePicker.dialogText)
            onClicked: fileDialog.open()
        }
    }
}
//! [filepicker]
