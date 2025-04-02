// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QPixmap>
#include <QString>

QT_BEGIN_NAMESPACE

enum class TranslationMarks { OnMark, OffMark, ObsoleteMark, DangerMark, WarningMark, EmptyMark };

const QString &settingsPrefix();
QString settingPath(const char *path);
bool isDarkMode();
QPixmap createMarkIcon(TranslationMarks mark, bool darkMode = isDarkMode());

QT_END_NAMESPACE

#endif // GLOBALS_H
