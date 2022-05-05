/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
