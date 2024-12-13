// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "globals.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QPainter>
#include <QStyleHints>

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

// Check for "Dark Mode", either system-wide or usage of a dark style
static bool isLight(const QColor &textColor)
{
    constexpr int DarkThreshold = 200;
    return textColor.red() > DarkThreshold && textColor.green() > DarkThreshold
            && textColor.blue() > DarkThreshold;
}

bool isDarkMode()
{
    return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark
            || isLight(QGuiApplication::palette().color(QPalette::WindowText));
}

QPixmap UnicodeIconGenerator::create(QChar unicode, Qt::GlobalColor color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setFont(*m_font);
    painter.setPen(color);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, unicode);
    painter.end();
    return pixmap;
}

UnicodeIconGenerator::UnicodeIconGenerator()
    : m_font(std::make_unique<QFont>())
{
    m_font->setBold(true);
    m_font->setPointSize(18);
}
