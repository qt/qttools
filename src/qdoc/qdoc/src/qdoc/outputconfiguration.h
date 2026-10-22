// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OUTPUTCONFIGURATION_H
#define OUTPUTCONFIGURATION_H

#include "location.h"

#include <QtCore/qhash.h>
#include <QtCore/qset.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class Config;

enum class Pipeline { Legacy, Template };

struct PipelineCapabilities
{
    Pipeline pipeline = Pipeline::Legacy;
    bool available = true;
    QSet<QString> supportedFormats;
    QSet<QString> brokenFormats;
    QHash<QString, QString> formatExtensions;
};

struct OutputConfigurationInput
{
    QStringList cliPipelines;
    QStringList cliFormats;
    QVector<PipelineCapabilities> pipelineCapabilities;
};

struct OutputSelection
{
    enum class Mode { Compatibility, New };

    struct Diagnostic
    {
        enum class Kind { Error, Notice };
        Kind kind = Kind::Error;
        Location location;
        QString message;
        QString details;
    };

    struct Format
    {
        QString format;
        bool requested = true;
        bool configured = false;
        Pipeline pipeline = Pipeline::Legacy;
        QString fileExtension;
        Location location;
    };

    Mode mode = Mode::Compatibility;
    QVector<Pipeline> pipelines;
    QVector<Format> formats;
    QVector<Diagnostic> diagnostics;

    [[nodiscard]] static OutputSelection resolve(const Config &config,
                                                 const OutputConfigurationInput &input = {});
    [[nodiscard]] bool hasErrors() const;
    [[nodiscard]] const Format *findFormat(const QString &format) const;
};

QT_END_NAMESPACE

#endif // OUTPUTCONFIGURATION_H
