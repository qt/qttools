// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
