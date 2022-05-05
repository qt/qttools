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

#include "stdinlistener.h"

#include "tracer.h"

#include <stdio.h>

QT_BEGIN_NAMESPACE

StdInListener::StdInListener(QObject *parent)
    : QSocketNotifier(fileno(stdin), QSocketNotifier::Read, parent)
{
    TRACE_OBJ
    connect(this, &QSocketNotifier::activated,
            this, &StdInListener::receivedData);
}

StdInListener::~StdInListener()
{
    TRACE_OBJ
}

void StdInListener::start()
{
    setEnabled(true);
}

void StdInListener::receivedData()
{
    TRACE_OBJ
    QByteArray ba;
    while (true) {
        const int c = getc(stdin);
        if (c == EOF) {
            setEnabled(false);
            break;
        }
        if (c == '\0')
            break;
        if (c)
            ba.append(char(c));
         if (c == '\n')
             break;
    }
    emit receivedCommand(QString::fromLocal8Bit(ba));
}

QT_END_NAMESPACE
