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
    RemoteControl();
    ~RemoteControl();

private:
    Ui::RemoteControlClass ui;
    QProcess *process;

private:
    void onLaunchClicked();
    void onIndexClicked();
    void onIdClicked();
    void onUrlClicked();

    void sendCommand(const QString &cmd);
};

#endif // REMOTECONTROL_H
