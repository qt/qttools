// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEMPLATEPACKAGE_H
#define TEMPLATEPACKAGE_H

#include "location.h"
#include "outputconfiguration.h"

#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class Config;

struct TemplatePackageInput
{
    QSet<QString> engineCapabilities;
    QString builtinRoot = QStringLiteral(":/qdoc/templates");
};

struct TemplatePackage
{
    using Diagnostic = OutputSelection::Diagnostic;

    static constexpr int supportedSchemaVersion = 1;
    static constexpr int providedTemplateApiVersion = 1;

    struct Template
    {
        QString path;
        Location location;
    };

    struct Format
    {
        QString format;
        QString extension;
        Location extensionLocation;
        QMap<QString, Template> templates;
        Location location;
        bool broken = false;
    };

    QString id;
    Location location;
    bool loaded = false;
    QStringList searchPaths;
    QVector<Format> formats;
    QVector<Diagnostic> diagnostics;

    [[nodiscard]] static QString builtinId();
    [[nodiscard]] static QString manifestFileName();
    [[nodiscard]] static QStringList roles();
    [[nodiscard]] static TemplatePackage resolve(const Config &config,
                                                 const TemplatePackageInput &input = {});
    [[nodiscard]] bool hasErrors() const;
    [[nodiscard]] const Format *findFormat(const QString &format) const;
    [[nodiscard]] PipelineCapabilities capabilities() const;
};

QT_END_NAMESPACE

#endif // TEMPLATEPACKAGE_H
