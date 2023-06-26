// Copyright (C) 2020 The Qt Company Ltd and/or its subsidiary(-ies).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"

#include "qdbusviewer.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>
#include <QtWidgets/QMessageBox>

#include <QtDBus/QDBusConnection>
#include <QtCore/QSettings>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , systemBusViewer(nullptr)
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *quitAction = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setMenuRole(QAction::QuitRole);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setMenuRole(QAction::AboutRole);
    QObject::connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    QAction *aboutQtAction = helpMenu->addAction(tr("About &Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    QObject::connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

    tabWidget = new QTabWidget;
    setCentralWidget(tabWidget);

    sessionBusViewer = new QDBusViewer(QDBusConnection::sessionBus());
    tabWidget->addTab(sessionBusViewer, tr("Session Bus"));

    QDBusConnection connection = QDBusConnection::systemBus();
    if (connection.isConnected()) {
        systemBusViewer = new QDBusViewer(connection);
        tabWidget->addTab(systemBusViewer, tr("System Bus"));
    }

    restoreSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::addCustomBusTab(const QString &busAddress)
{
    QDBusConnection connection = QDBusConnection::connectToBus(busAddress, "QDBusViewer");
    if (connection.isConnected()) {
        QDBusViewer *customBusViewer = new QDBusViewer(connection);
        tabWidget->addTab(customBusViewer, tr("Custom Bus"));
    }
}

void MainWindow::about()
{
    QMessageBox box(this);

    box.setText(tr("<center><img src=\":/qt-project.org/qdbusviewer/images/qdbusviewer-128.png\">"
                   "<h3>%1</h3>"
                   "<p>Version %2</p></center>"
                   "<p>Copyright (C) %3 The Qt Company Ltd.</p>")
                        .arg(tr("D-Bus Viewer"), QLatin1String(QT_VERSION_STR), u"2023"_s));
    box.setWindowTitle(tr("D-Bus Viewer"));
    box.exec();
}

static inline QString windowGeometryKey()
{
    return u"WindowGeometry"_s;
}

static inline QString sessionTabGroup()
{
    return u"SessionTab"_s;
}

static inline QString systemTabGroup()
{
    return u"SystemTab"_s;
}

void MainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue(windowGeometryKey(), saveGeometry());

    settings.beginGroup(sessionTabGroup());
    sessionBusViewer->saveState(&settings);
    settings.endGroup();

    if (systemBusViewer) {
        settings.beginGroup(systemTabGroup());
        systemBusViewer->saveState(&settings);
        settings.endGroup();
    }
}

void MainWindow::restoreSettings()
{
    QSettings settings;

    restoreGeometry(settings.value(windowGeometryKey()).toByteArray());

    settings.beginGroup(sessionTabGroup());
    sessionBusViewer->restoreState(&settings);
    settings.endGroup();

    if (systemBusViewer) {
        settings.beginGroup(systemTabGroup());
        systemBusViewer->restoreState(&settings);
        settings.endGroup();
    }
}
