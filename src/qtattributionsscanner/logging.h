// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
