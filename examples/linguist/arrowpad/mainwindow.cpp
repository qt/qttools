// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "arrowpad.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
//! [0]
    arrowPad = new ArrowPad;
//! [0]
    setCentralWidget(arrowPad);

//! [1]
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &MainWindow::close);
//! [1]

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);
}
