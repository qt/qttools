// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [3]
label->setText(tr("F\374r \310lise"));
//! [3]
//! [4]
menuBar: MenuBar {
    Menu {
        title: qsTr("&File")
        Action { text: qsTr("&New...") }
        Action { text: qsTr("&Open...") }
        Action { text: qsTr("&Save") }
        Action { text: qsTr("Save &As...") }
        MenuSeparator { }
        Action { text: qsTr("&Quit") }
    }
    Menu {
        title: qsTr("&Edit")
        Action { text: qsTr("Cu&t") }
        Action { text: qsTr("&Copy") }
        Action { text: qsTr("&Paste") }
    }
    Menu {
        title: qsTr("&Help")
        Action { text: qsTr("&About") }
    }
}

//! [4]
//! [5]
/*
  TRANSLATOR MainWindow

  This class contains the main application window interface.
  All menu items and toolbar buttons are defined here.
*/
class MainWindow : public QMainWindow
{
  // ... translations for this context
};
//! [5]
