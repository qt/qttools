// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "globals.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyleHints>

namespace {
// Check for "Dark Mode", either system-wide or usage of a dark style
static bool isLight(const QColor &textColor)
{
    constexpr int DarkThreshold = 200;
    return textColor.red() > DarkThreshold && textColor.green() > DarkThreshold
            && textColor.blue() > DarkThreshold;
}
} // namespace

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

const QString &settingsPrefix()
{

    static QString prefix =
            QString::number(QT_VERSION_MAJOR) + u'.' + QString::number(QT_VERSION_MINOR) + u'/';
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

QPixmap createMarkIcon(TranslationMarks mark, bool darkMode)
{
    const QString prefix = darkMode ? ":/images/darkmarks"_L1 : ":/images/lightmarks"_L1;
    switch (mark) {
    case TranslationMarks::OnMark:
        return QPixmap(prefix + "/on-mark"_L1)
                .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case TranslationMarks::OffMark:
        return QPixmap(prefix + "/off-mark"_L1)
                .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case TranslationMarks::ObsoleteMark:
        return QPixmap(prefix + "/obsolete-mark"_L1)
                .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case TranslationMarks::DangerMark:
        return QPixmap(prefix + "/danger-mark"_L1)
                .scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case TranslationMarks::WarningMark:
        return QPixmap(prefix + "/warning-mark"_L1)
                .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    case TranslationMarks::EmptyMark:
        return QPixmap(prefix + "/empty-mark"_L1)
                .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    };
    Q_UNREACHABLE_RETURN({});
}

QT_END_NAMESPACE
