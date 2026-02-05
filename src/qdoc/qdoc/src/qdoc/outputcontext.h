// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OUTPUTCONTEXT_H
#define OUTPUTCONTEXT_H

#include "outputdirectory.h"

#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Config;

struct OutputContext
{
    OutputDirectory outputDir;
    QString project;
    QString subdir;
    QString fileExtension;
    QHash<QString, QString> outputPrefixes;
    QHash<QString, QString> outputSuffixes;
    bool useSubdirs{true};
    bool noLinkErrors{false};
    bool autolinkErrors{false};

    [[nodiscard]] static OutputContext fromConfig(const Config &config,
                                                   const QString &format);
    [[nodiscard]] QString outputPrefix(const QString &nodeType) const;
    [[nodiscard]] QString outputSuffix(const QString &nodeType) const;
};

QT_END_NAMESPACE

#endif // OUTPUTCONTEXT_H

