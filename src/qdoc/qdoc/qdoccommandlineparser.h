// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOCCOMMANDLINEPARSER_H
#define QDOCCOMMANDLINEPARSER_H

#include <QtCore/qcommandlineparser.h>

QT_BEGIN_NAMESPACE

struct QDocCommandLineParser : public QCommandLineParser
{
    QDocCommandLineParser();
    void process(const QStringList &arguments);

    QCommandLineOption defineOption, dependsOption, highlightingOption;
    QCommandLineOption showInternalOption, redirectDocumentationToDevNullOption;
    QCommandLineOption noExamplesOption, indexDirOption, installDirOption;
    QCommandLineOption outputDirOption, outputFormatOption;
    QCommandLineOption noLinkErrorsOption, autoLinkErrorsOption, debugOption, atomsDumpOption;
    QCommandLineOption prepareOption, generateOption, logProgressOption, singleExecOption;
    QCommandLineOption includePathOption, includePathSystemOption, frameworkOption;
    QCommandLineOption timestampsOption, useDocBookExtensions;
};

QT_END_NAMESPACE

#endif // QDOCCOMMANDLINEPARSER_H
