// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include "ui_remotecontrol.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QProcess;
QT_END_NAMESPACE

class RemoteControl : public QMainWindow
{
    Q_OBJECT

public:
    RemoteControl(QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~RemoteControl();

private:
    Ui::RemoteControlClass ui;
    QProcess *process;

private slots:
    void on_launchButton_clicked();
    void on_actionQuit_triggered();
    void on_indexButton_clicked();
    void on_identifierButton_clicked();
    void on_urlButton_clicked();
    void on_syncContentsButton_clicked();
    void on_contentsCheckBox_toggled(bool checked);
    void on_indexCheckBox_toggled(bool checked);
    void on_bookmarksCheckBox_toggled(bool checked);
    void helpViewerClosed();

    void sendCommand(const QString &cmd);
};

#endif // REMOTECONTROL_H
