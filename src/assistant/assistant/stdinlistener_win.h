// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef STDINLISTENER_WIN_H
#define STDINLISTENER_WIN_H

#include <windows.h>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

class StdInListener : public QThread
{
    Q_OBJECT

public:
    StdInListener(QObject *parent);
    ~StdInListener();

signals:
    void receivedCommand(const QString &cmd);

private:
    void run() override;
    bool ok;
};

QT_END_NAMESPACE

#endif
