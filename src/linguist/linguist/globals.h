// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QFont>
#include <QPixmap>
#include <QString>

#include <memory>

const QString &settingsPrefix();
QString settingPath(const char *path);
bool isDarkMode();

class UnicodeIconGenerator
{
public:
    QPixmap create(QChar unicode, Qt::GlobalColor color);

    UnicodeIconGenerator();

private:
    std::unique_ptr<QFont> m_font;
};

#endif // GLOBALS_H
