// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "globals.h"

const QString &settingsPrefix()
{
    static QString prefix = QString(QLatin1String("%1.%2/"))
        .arg((QT_VERSION >> 16) & 0xff)
        .arg((QT_VERSION >> 8) & 0xff);
    return prefix;
}

QString settingPath(const char *path)
{
    return settingsPrefix() + QLatin1String(path);
}
