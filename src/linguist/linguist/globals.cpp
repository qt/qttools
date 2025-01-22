// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "globals.h"

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QStyleHints>
#include <QPalette>
#include <QPainter>

namespace {
// Check for "Dark Mode", either system-wide or usage of a dark style
static bool isLight(const QColor &textColor)
{
    constexpr int DarkThreshold = 200;
    return textColor.red() > DarkThreshold && textColor.green() > DarkThreshold
            && textColor.blue() > DarkThreshold;
}
} // namespace

QT_BEGIN_NAMESPACE

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

bool isDarkMode()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark
            || isLight(QGuiApplication::palette().color(QPalette::WindowText));
}

QPixmap MarkIcon::create(TranslationMarks mark, bool darkMode)
{
    switch (mark) {
    case onMark:
        return darkMode ? createInternal(QChar(0x2713), QColor(Qt::darkGreen).lighter())
                        : createInternal(QChar(0x2713), Qt::darkGreen);
    case offMark:
        return darkMode ? createInternal(u'?', Qt::yellow) : createInternal(u'?', Qt::darkYellow);
    case obsoleteMark:
        return createInternal(QChar(0x2713), Qt::gray);
    case dangerMark:
        return createInternal(u'!', Qt::red);
    case warningMark:
        return darkMode ? createInternal(QChar(0x2713), Qt::yellow)
                        : createInternal(QChar(0x2713), Qt::darkYellow);
    case emptyMark:
        return darkMode ? createInternal(u'?', Qt::white) : createInternal(u'?', Qt::darkBlue);
    };
    Q_UNREACHABLE_RETURN({});
}

QPixmap MarkIcon::createInternal(QChar unicode, const QColor &color)
{
    static QFont font = getFont();
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, unicode);
    painter.end();
    return pixmap;
}

const QFont &MarkIcon::getFont()
{
    static QFont font;
    font.setBold(true);
    font.setPointSize(14);
    return font;
}

QT_END_NAMESPACE
