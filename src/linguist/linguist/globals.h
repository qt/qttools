// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QPixmap>
#include <QString>

QT_BEGIN_NAMESPACE

const QString &settingsPrefix();
QString settingPath(const char *path);

bool isDarkMode();

class QFont;

class MarkIcon
{
public:
    enum TranslationMarks { onMark, offMark, obsoleteMark, dangerMark, warningMark, emptyMark };

    static QPixmap create(TranslationMarks mark, bool darkMode = isDarkMode());

private:
    static QPixmap createInternal(QChar unicode, const QColor &color);
    static const QFont &getFont();
};

QT_END_NAMESPACE

#endif // GLOBALS_H
