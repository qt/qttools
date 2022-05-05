/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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
