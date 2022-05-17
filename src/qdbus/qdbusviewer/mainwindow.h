// Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QTabWidget)

class QDBusViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void addCustomBusTab(const QString &bus);

private slots:
    void about();

private:
    void saveSettings();
    void restoreSettings();

    QTabWidget *tabWidget;
    QDBusViewer *sessionBusViewer;
    QDBusViewer *systemBusViewer;
};

#endif // MAINWINDOW_H
