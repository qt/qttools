// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LOGGING_H
#define LOGGING_H

#include <QtCore/qcoreapplication.h>

enum LogLevel {
    VerboseLog,
    NormalLog,
    SilentLog
};

static QString tr(const char *key) {
    return QCoreApplication::translate("qtattributionsscanner", key);
}

#endif // LOGGING_H
