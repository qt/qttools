// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "assistant.h"
#include "findfiledialog.h"
#include "mainwindow.h"
#include "textedit.h"

#include <QAction>
#include <QApplication>
#include <QLibraryInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

using namespace Qt::StringLiterals;

// ![0]
MainWindow::MainWindow()
    : textViewer(new TextEdit)
    , assistant(new Assistant)
{
// ![0]
    textViewer->setContents(QLibraryInfo::path(QLibraryInfo::ExamplesPath)
                            + "/assistant/simpletextviewer/documentation/intro.html"_L1);
    setCentralWidget(textViewer);

    createActions();
    createMenus();

    setWindowTitle(tr("Simple Text Viewer"));
    resize(750, 400);

    connect(textViewer, &TextEdit::fileNameChanged, this, &MainWindow::updateWindowTitle);
// ![1]
}
//! [1]

//! [2]
void MainWindow::closeEvent(QCloseEvent *)
{
    delete assistant;
}
//! [2]

void MainWindow::updateWindowTitle(const QString &fileName)
{
    setWindowTitle(tr("Simple Text Viewer - %1").arg(fileName));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Text Viewer"),
                       tr("This example demonstrates how to use\n"
                          "Qt Assistant as help system for your\n"
                          "own application."));
}

//! [3]
void MainWindow::showDocumentation()
{
    assistant->showDocumentation("index.html");
}
//! [3]

void MainWindow::open()
{
    FindFileDialog dialog(textViewer, assistant);
    dialog.exec();
}

//! [4]
void MainWindow::createActions()
{
    assistantAct = new QAction(tr("Help Contents"), this);
    assistantAct->setShortcut(QKeySequence::HelpContents);
    connect(assistantAct, &QAction::triggered, this, &MainWindow::showDocumentation);
//! [4]

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    clearAct = new QAction(tr("&Clear"), this);
    clearAct->setShortcut(tr("Ctrl+C"));
    connect(clearAct, &QAction::triggered, textViewer, &QTextEdit::clear);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, &QAction::triggered, QApplication::aboutQt);
//! [5]
}
//! [5]

void MainWindow::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(clearAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(assistantAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(helpMenu);
}
