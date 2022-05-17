// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
