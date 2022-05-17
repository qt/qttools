// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "mainwindow.h"
#include "printpanel.h"

MainWindow::MainWindow()
{
    printPanel = new PrintPanel;
    setCentralWidget(printPanel);

    createActions();
    createMenus();

//! [0]
    setWindowTitle(tr("Troll Print 1.0"));
//! [0]
}

void MainWindow::about()
{
    QMessageBox::information(this, tr("About Troll Print 1.0"),
                      tr("Troll Print 1.0.\n\n"
                      "Copyright 1999 Software, Inc."));
}

//! [1]
void MainWindow::createActions()
{
//! [2]
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q", "Quit"));
//! [2]
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setShortcut(Qt::Key_F1);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
//! [1] //! [3]
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}
//! [3]
