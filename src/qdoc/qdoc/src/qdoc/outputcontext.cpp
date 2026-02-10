// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "outputcontext.h"

#include "config.h"
#include "location.h"

#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class OutputContext
    \internal
    \brief Bundles output configuration without static variables.

    OutputContext captures all the configuration state that generators
    need for output, replacing Generator's 19 static variables with
    explicit, injectable state.

    This enables:
    \list
        \li Testability - configurations can be created for tests.
        \li Reusability - HrefResolver, IndexWriter can use the same context.
        \li Statelessness - no global mutable state between runs.
    \endlist

    \section1 Relationship to Generator Statics

    | OutputContext field | Generator static |
    |---------------------|------------------|
    | outputDir           | s_outDir         |
    | project             | s_project        |
    | subdir              | s_outSubdir      |
    | outputPrefixes      | s_outputPrefixes |
    | outputSuffixes      | s_outputSuffixes |
    | useSubdirs          | s_useOutputSubdirs |

    \sa DocumentWriter, FileDocumentWriter
*/

/*!
    Creates an OutputContext from the given \a config for the specified output
    \a format.

    This method extracts all relevant configuration values that generators need,
    centralizing the logic that was previously scattered across Generator's
    static initialization.
*/
OutputContext OutputContext::fromConfig(const Config &config, const QString &format)
{
    QString project = config.get(CONFIG_PROJECT).asString();

    // Determine output directory
    QString outDir = config.getOutputDir(format);
    if (outDir.isEmpty())
        outDir = config.getOutputDir(); // Fallback to default

    QString subdir;
    OutputDirectory outputDir{outDir};
    if (!outDir.isEmpty()) {
        outputDir = OutputDirectory::ensure(outDir, Location{});
        subdir = outDir.mid(outDir.lastIndexOf('/'_L1) + 1);
    }

    // Extract output prefixes
    QHash<QString, QString> prefixes;
    QStringList prefixItems = config.get(CONFIG_OUTPUTPREFIXES).asStringList();
    for (const auto &prefix : prefixItems) {
        prefixes[prefix] =
                config.get(CONFIG_OUTPUTPREFIXES + Config::dot + prefix).asString();
    }
    // Default QML prefix if not specified
    if (!prefixItems.contains(u"QML"_s))
        prefixes[u"QML"_s] = u"qml-"_s;

    // Extract output suffixes
    QHash<QString, QString> suffixes;
    for (const auto &suffix : config.get(CONFIG_OUTPUTSUFFIXES).asStringList()) {
        suffixes[suffix] =
                config.get(CONFIG_OUTPUTSUFFIXES + Config::dot + suffix).asString();
    }

    bool useSubdirs = !config.get(format + Config::dot + "nosubdirs").asBool();
    bool noLinkErrors = config.get(CONFIG_NOLINKERRORS).asBool();
    bool autolinkErrors = config.get(CONFIG_AUTOLINKERRORS).asBool();

    return OutputContext{
        std::move(outputDir),
        std::move(project),
        std::move(subdir),
        QString{},  // Is set by caller based on generator
        std::move(prefixes),
        std::move(suffixes),
        useSubdirs,
        noLinkErrors,
        autolinkErrors
    };
}

/*!
    Returns the output prefix for the given \a nodeType.
    Common values: "QML" -> "qml-", "CPP" -> "cpp-".
*/
QString OutputContext::outputPrefix(const QString &nodeType) const
{
    return outputPrefixes.value(nodeType);
}

/*!
    Returns the output suffix for the given \a nodeType.
*/
QString OutputContext::outputSuffix(const QString &nodeType) const
{
    return outputSuffixes.value(nodeType);
}

QT_END_NAMESPACE

