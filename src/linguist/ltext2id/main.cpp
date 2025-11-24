// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "filetransformer.h"
#include "fileverifier.h"
#include "utils.h"
#include "recorddirectory.h"
#include <translator.h>
#include <trlib/trparser.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QDirIterator>
#include <iostream>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ltext2id"_L1);
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));

    QCommandLineParser parser;
    parser.setApplicationDescription(
            "ltext2id is part of the Qt Linguist toolchain. It transforms\n"
            "a project that uses text-based translations into using id-based\n"
            "translations. It extracts the source and translation files (TS\n"
            "files) from the given project root by traversing the\n"
            "directories recursively. The code will be updated to use\n"
            "id-based translations. Also, the TS files are updated accordingly.\n"
            "Please make a backup of your code base before running ltext2id.\n"
            "For best results, include the sources along with TS files to get\n"
            "consistent transformation across sources and TS files."_L1);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption noAutoIdOption(
            "no-auto-id"_L1,
            "Do not auto generate IDs when missing meta string IDs. Ignores the\n"
            "translation functions without meta IDs."_L1);
    parser.addOption(noAutoIdOption);

    QCommandLineOption noLabelsOption(
            "no-labels"_L1,
            "Do not generate labels for id-based translations. The default\n"
            "behavior is to use message contexts as labels, to preserve\n"
            "the same visualization form in Linguist."_L1);
    parser.addOption(noLabelsOption);

    QCommandLineOption onlyMetaIdOption(
            "only-meta-id"_L1,
            "Only generate ID suggestions in form of meta strings. Does not\n"
            "perform the actual transformation into ID based."_L1);
    parser.addOption(onlyMetaIdOption);

    QCommandLineOption quietOption("quiet"_L1, "Do not print progress output"_L1);
    parser.addOption(quietOption);

    QCommandLineOption sortMessagesOption(
            "sort-messages"_L1, "Sort messages in a context alphabetically in TS files."_L1);
    parser.addOption(sortMessagesOption);

    QCommandLineOption sourceUtf16Option(
            "source-utf16"_L1,
            "If the encoding of the source files is UTF16 (default is UTF-8)"_L1);
    parser.addOption(sourceUtf16Option);

    parser.addPositionalArgument("project-root"_L1,
                                 "Path to the project root directory or file"_L1);

    parser.process(app);

    const QStringList positionalArgs = parser.positionalArguments();
    if (positionalArgs.isEmpty()) {
        std::cerr << "ltext2id error: a project root must be specified\n";
        parser.showHelp(1);
    }

    const QString projectRoot = positionalArgs.first();
    const bool labels = !parser.isSet(noLabelsOption);
    const bool sortMessages = parser.isSet(sortMessagesOption);
    const bool utf16 = parser.isSet(sourceUtf16Option);
    const bool quiet = parser.isSet(quietOption);
    const bool autoId = !parser.isSet(noAutoIdOption);
    const bool onlyId = parser.isSet(onlyMetaIdOption);
    QFileInfo rootInfo(projectRoot);
    if (!rootInfo.exists()) {
        std::cerr << "ltext2id error: invalid project root\n";
        parser.showHelp(1);
    }

    QStringList sources;
    QStringList cSources;
    QStringList translations;

    auto routeFile = [&sources, &cSources, &translations](QString file) {
        file = QFileInfo(file).absoluteFilePath();
        if (QString ext = QFileInfo(file).suffix(); FileTransformer::cppExtensions.contains(ext)) {
            cSources << file;
            sources << std::move(file);
        } else if (FileTransformer::otherExtensions.contains(ext))
            sources << std::move(file);
        else if (ext == "ts" || ext == "TS")
            translations << std::move(file);
    };

    if (rootInfo.isDir()) {
        QDirIterator it(projectRoot, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (it.hasNext())
            routeFile(it.next());
    } else {
        routeFile(projectRoot);
    }

    ConversionData cd;
    Translator tor;
    cd.m_projectRoots = { std::move(projectRoot) };
    cd.m_allCSources = Utils::getIncludeOptions(rootInfo, cSources);
    cd.m_sourceIsUtf16 = utf16;
    processSources(tor, sources, cd);

    RecordDirectory records;
    for (const TranslatorMessage &msg : tor.messages())
        if (msg.context().isEmpty())
            records.recordExistingId(msg.id());
    for (const TranslatorMessage &msg : tor.messages())
        if (!msg.context().isEmpty() && (autoId || !msg.extra(meta_id_key).isEmpty()))
            records.recordMessage(msg);

    bool fail = false;
    FileTransformer transformer(records, labels, quiet);
    FileVerifier verifier(records, quiet);

    if (onlyId) {
        transformer.generateMetaIds();
        fail = !transformer.updateTsFiles(translations);
    } else {
        transformer.transformUi();
        transformer.transformSources();

        fail = !transformer.transformTsFiles(translations, sortMessages);
        verifier.verifySources(sources, cd);
        fail |= !records.errors().empty();
        if (fail)
            Utils::printErr("ltext2id failed to migrate some translations. Please look for "
                            "the comments from ltext2id annotated by '//ltext2id error:' in "
                            "your source code to find the failed transformations and fix them "
                            "manually as instructed."_L1);
    }

    std::cout << "ltext2id: finished migration from text-based to id-based translation.\n";
    return fail ? 1 : 0;
}
