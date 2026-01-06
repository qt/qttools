// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TRANSLATIONSETTINGS_H
#define TRANSLATIONSETTINGS_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class TranslationSettings
{
public:
    static constexpr int DefaultMaxRetries = 10;
    static constexpr int DefaultMaxConcurrentBatches = 6;
    static constexpr int DefaultTransferTimeoutMs = 5 * 60 * 1000;
    static constexpr int DefaultMaxBatchSize = 20;
    static constexpr double DefaultTemperature = 0.05;
    static constexpr int DefaultMaxJsonFormatTries = 3;
    static constexpr int DefaultOllamaWakeUpTimeoutMs = 4 * 60 * 1000;

    static constexpr const char *MaxRetriesKey = "MachineTranslation/MaxRetries";
    static constexpr const char *MaxConcurrentBatchesKey =
            "MachineTranslation/MaxConcurrentBatches";
    static constexpr const char *TransferTimeoutMsKey = "MachineTranslation/TransferTimeoutMs";
    static constexpr const char *MaxBatchSizeKey = "MachineTranslation/MaxBatchSize";
    static constexpr const char *TemperatureKey = "MachineTranslation/Temperature";
    static constexpr const char *MaxJsonFormatTriesKey = "MachineTranslation/MaxJsonFormatTries";
    static constexpr const char *OllamaWakeUpTimeoutMsKey =
            "MachineTranslation/OllamaWakeUpTimeoutMs";

    static int maxRetries();
    static int maxConcurrentBatches();
    static int transferTimeoutMs();
    static int maxBatchSize();
    static double temperature();
    static int maxJsonFormatTries();
    static int ollamaWakeUpTimeoutMs();

    static void setMaxRetries(int value);
    static void setMaxConcurrentBatches(int value);
    static void setTransferTimeoutMs(int value);
    static void setMaxBatchSize(int value);
    static void setTemperature(double value);
    static void setMaxJsonFormatTries(int value);
    static void setOllamaWakeUpTimeoutMs(int value);

    static void resetToDefaults();

private:
    TranslationSettings() = delete;
};

QT_END_NAMESPACE

#endif // TRANSLATIONSETTINGS_H
