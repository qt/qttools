// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translationsettings.h"

#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>

namespace {

QString settingPath(const char *path)
{
    static const QString prefix =
            QString::number(QT_VERSION_MAJOR) + u'.' + QString::number(QT_VERSION_MINOR) + u'/';
    return prefix + QLatin1String(path);
}

} // namespace

QT_BEGIN_NAMESPACE

int TranslationSettings::maxRetries()
{
    return QSettings().value(settingPath(MaxRetriesKey), DefaultMaxRetries).toInt();
}

int TranslationSettings::maxConcurrentBatches()
{
    return QSettings()
            .value(settingPath(MaxConcurrentBatchesKey), DefaultMaxConcurrentBatches)
            .toInt();
}

int TranslationSettings::transferTimeoutMs()
{
    return QSettings().value(settingPath(TransferTimeoutMsKey), DefaultTransferTimeoutMs).toInt();
}

int TranslationSettings::maxBatchSize()
{
    return QSettings().value(settingPath(MaxBatchSizeKey), DefaultMaxBatchSize).toInt();
}

double TranslationSettings::temperature()
{
    return QSettings().value(settingPath(TemperatureKey), DefaultTemperature).toDouble();
}

int TranslationSettings::maxJsonFormatTries()
{
    return QSettings().value(settingPath(MaxJsonFormatTriesKey), DefaultMaxJsonFormatTries).toInt();
}

int TranslationSettings::ollamaWakeUpTimeoutMs()
{
    return QSettings()
            .value(settingPath(OllamaWakeUpTimeoutMsKey), DefaultOllamaWakeUpTimeoutMs)
            .toInt();
}

void TranslationSettings::setMaxRetries(int value)
{
    QSettings().setValue(settingPath(MaxRetriesKey), value);
}

void TranslationSettings::setMaxConcurrentBatches(int value)
{
    QSettings().setValue(settingPath(MaxConcurrentBatchesKey), value);
}

void TranslationSettings::setTransferTimeoutMs(int value)
{
    QSettings().setValue(settingPath(TransferTimeoutMsKey), value);
}

void TranslationSettings::setMaxBatchSize(int value)
{
    QSettings().setValue(settingPath(MaxBatchSizeKey), value);
}

void TranslationSettings::setTemperature(double value)
{
    QSettings().setValue(settingPath(TemperatureKey), value);
}

void TranslationSettings::setMaxJsonFormatTries(int value)
{
    QSettings().setValue(settingPath(MaxJsonFormatTriesKey), value);
}

void TranslationSettings::setOllamaWakeUpTimeoutMs(int value)
{
    QSettings().setValue(settingPath(OllamaWakeUpTimeoutMsKey), value);
}

void TranslationSettings::resetToDefaults()
{
    QSettings settings;
    settings.remove(settingPath(MaxRetriesKey));
    settings.remove(settingPath(MaxConcurrentBatchesKey));
    settings.remove(settingPath(TransferTimeoutMsKey));
    settings.remove(settingPath(MaxBatchSizeKey));
    settings.remove(settingPath(TemperatureKey));
    settings.remove(settingPath(MaxJsonFormatTriesKey));
    settings.remove(settingPath(OllamaWakeUpTimeoutMsKey));
}

QT_END_NAMESPACE
