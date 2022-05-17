// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE
class ArrowPad;

//! [0]
class MainWindow : public QMainWindow
//! [0] //! [1]
{
    Q_OBJECT
//! [1]

public:
    MainWindow();

private:
    ArrowPad *arrowPad;
    QMenu *fileMenu;
    QAction *exitAct;
};

#endif
