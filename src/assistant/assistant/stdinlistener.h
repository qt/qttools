// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef STDINLISTENER_H
#define STDINLISTENER_H

#include <QtCore/QSocketNotifier>

QT_BEGIN_NAMESPACE

class StdInListener : public QSocketNotifier
{
    Q_OBJECT

public:
    StdInListener(QObject *parent);
    ~StdInListener();

public slots:
    void start();

signals:
    void receivedCommand(const QString &cmd);

private slots:
    void receivedData();
};

QT_END_NAMESPACE

#endif
