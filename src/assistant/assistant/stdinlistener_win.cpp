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

#include "stdinlistener_win.h"

#include "tracer.h"

QT_BEGIN_NAMESPACE

StdInListener::StdInListener(QObject *parent)
    : QThread(parent)
{
    TRACE_OBJ
}

StdInListener::~StdInListener()
{
    TRACE_OBJ
    terminate();
    wait();
}

void StdInListener::run()
{
    TRACE_OBJ
    bool ok = true;
    char chBuf[4096];
    DWORD dwRead;
    HANDLE hStdinDup;

    const HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        return;

    DuplicateHandle(GetCurrentProcess(), hStdin,
        GetCurrentProcess(), &hStdinDup,
        0, false, DUPLICATE_SAME_ACCESS);

    CloseHandle(hStdin);

    while (ok) {
        ok = ReadFile(hStdinDup, chBuf, sizeof(chBuf), &dwRead, NULL);
        if (ok && dwRead != 0)
            emit receivedCommand(QString::fromLocal8Bit(chBuf, dwRead));
    }
}

QT_END_NAMESPACE
