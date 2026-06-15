// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>

#include <iostream>

QT_USE_NAMESPACE

using namespace Qt::Literals::StringLiterals;

static int usage(const QStringList &args)
{
    Q_UNUSED(args);

    QString loaders;
    QString line("    %1 - %2\n"_L1);
    for (const Translator::FileFormat &format : std::as_const(Translator::registeredFileFormats()))
        loaders += line.arg(format.extension, -5).arg(format.description());

    std::cout << qPrintable(QStringLiteral("\nUsage:\n"
        "    lconvert [options] <infile> [<infile>...]\n\n"
        "lconvert is part of Qt's Linguist tool chain. It can be used as a\n"
        "stand-alone tool to convert and filter translation data files.\n"
        "The following file formats are supported:\n\n%1\n"
        "If multiple input files are specified, they are merged with\n"
        "translations from later files taking precedence.\n\n"
        "Options:\n"
        "    -h\n"
        "    -help  Display this information and exit.\n\n"
        "    -i <infile>\n"
        "    -input-file <infile>\n"
        "           Specify input file. Use if <infile> might start with a dash.\n"
        "           This option can be used several times to merge inputs.\n"
        "           May be '-' (standard input) for use in a pipe.\n\n"
        "    -o <outfile>\n"
        "    -output-file <outfile>\n"
        "           Specify output file. Default is '-' (standard output).\n\n"
        "    -if <informat>\n"
        "    -input-format <format>\n"
        "           Specify input format for subsequent <infile>s.\n"
        "           The format is auto-detected from the file name and defaults to 'ts'.\n\n"
        "    -of <outformat>\n"
        "    -output-format <outformat>\n"
        "           Specify output format. See -if.\n\n"
        "    -drop-tags <regexp>\n"
        "           Drop named extra tags when writing TS or XLIFF files.\n"
        "           May be specified repeatedly.\n\n"
        "    -drop-translations\n"
        "           Drop existing translations and reset the status to 'unfinished'.\n"
        "           Note: this implies --no-obsolete.\n\n"
        "    -source-language <language>[_<region>]\n"
        "           Specify/override the language of the source strings. Defaults to\n"
        "           POSIX if not specified and the file does not name it yet.\n\n"
        "    -target-language <language>[_<region>]\n"
        "           Specify/override the language of the translation.\n"
        "           The target language is guessed from the file name if this option\n"
        "           is not specified and the file contents name no language yet.\n\n"
        "    -no-obsolete\n"
        "           Drop obsolete messages.\n\n"
        "    -no-finished\n"
        "           Drop finished messages.\n\n"
        "    -no-untranslated\n"
        "           Drop untranslated messages.\n\n"
        "    -sort-contexts\n"
        "           Sort contexts in output TS file alphabetically.\n\n"
        "    -sort-messages\n"
        "           Sort messages in a context alphabetically in TS files.\n\n"
        "    -locations {absolute|relative|none}\n"
        "           Override how source code references are saved in TS files.\n"
        "           Default is absolute.\n\n"
        "    -no-ui-lines\n"
        "           Drop line numbers from references to UI files.\n\n"
        "    -pluralonly\n"
        "           Drop non-plural form messages.\n\n"
        "    -verbose\n"
        "           be a bit more verbose\n\n"
        "Long options can be specified with only one leading dash, too.\n\n"
        "Return value:\n"
        "    0 on success\n"
        "    1 on command line parse failures\n"
        "    2 on read failures\n"
        "    3 on write failures\n").arg(loaders));
    return 1;
}

struct File
{
    QString name;
    QString format;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
#ifndef QT_BOOTSTRAPPED
#ifndef Q_OS_WIN32
    QTranslator translator;
    QTranslator qtTranslator;
    QString resourceDir = QLibraryInfo::paths(QLibraryInfo::TranslationsPath).value(0);
    if (translator.load("linguist_en"_L1, resourceDir)
        && qtTranslator.load("qt_en"_L1, resourceDir)) {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
    }
#endif // Q_OS_WIN32
#endif

    QStringList args = app.arguments();
    QList<File> inFiles;
    QString inFormat("auto"_L1);
    QString outFileName;
    QString outFormat("auto"_L1);
    QString targetLanguage;
    QString sourceLanguage;
    bool dropTranslations = false;
    bool noObsolete = false;
    bool noFinished = false;
    bool noUntranslated = false;
    bool verbose = false;
    bool noUiLines = false;
    bool pluralOnly = false;
    Translator::LocationsType locations = Translator::DefaultLocations;

    ConversionData cd;
    Translator tr;

    for (int i = 1; i < args.size(); ++i) {
        const QString &arg = args.at(i);
        if (arg.startsWith("--"_L1))
            args[i].remove(0, 1);
        if (arg == "-o"_L1 || arg == "-output-file"_L1) {
            if (++i >= args.size())
                return usage(args);
            outFileName = args[i];
        } else if (arg == "-of"_L1 || arg == "-output-format"_L1) {
            if (++i >= args.size())
                return usage(args);
            outFormat = args[i];
        } else if (arg == "-i"_L1 || arg == "-input-file"_L1) {
            if (++i >= args.size())
                return usage(args);
            File file;
            file.name = args[i];
            file.format = inFormat;
            inFiles.append(file);
        } else if (arg == "-if"_L1 || arg == "-input-format"_L1) {
            if (++i >= args.size())
                return usage(args);
            inFormat = args[i];
        } else if (arg == "-drop-tag"_L1 || arg == "-drop-tags"_L1) {
            if (++i >= args.size())
                return usage(args);
            cd.m_dropTags.append(args[i]);
        } else if (arg == "-drop-translations"_L1) {
            dropTranslations = true;
        } else if (arg == "-target-language"_L1) {
            if (++i >= args.size())
                return usage(args);
            targetLanguage = args[i];
        } else if (arg == "-source-language"_L1) {
            if (++i >= args.size())
                return usage(args);
            sourceLanguage = args[i];
        } else if (arg.startsWith("-h"_L1)) {
            usage(args);
            return 0;
        } else if (arg == "-no-obsolete"_L1) {
            noObsolete = true;
        } else if (arg == "-no-finished"_L1) {
            noFinished = true;
        } else if (arg == "-no-untranslated"_L1) {
            noUntranslated = true;
        } else if (arg == "-sort-contexts"_L1) {
            cd.m_sortContexts = true;
        } else if (arg == "-sort-messages"_L1) {
            cd.m_sortMessages = true;
        } else if (arg == "-locations"_L1) {
            if (++i >= args.size())
                return usage(args);
            if (args[i] == "none"_L1)
                locations = Translator::NoLocations;
            else if (args[i] == "relative"_L1)
                locations = Translator::RelativeLocations;
            else if (args[i] == "absolute"_L1)
                locations = Translator::AbsoluteLocations;
            else
                return usage(args);
        } else if (arg == "-no-ui-lines"_L1) {
            noUiLines = true;
        } else if (arg == "-pluralonly"_L1) {
            pluralOnly = true;
        } else if (arg == "-verbose"_L1) {
            verbose = true;
        } else if (arg.startsWith(u'-')) {
            return usage(args);
        } else {
            File file;
            file.name = args[i];
            file.format = inFormat;
            inFiles.append(file);
        }
    }

    if (inFiles.isEmpty())
        return usage(args);

    tr.setLanguageCode(Translator::guessLanguageCodeFromFileName(inFiles[0].name));

    if (!tr.load(inFiles[0].name, cd, inFiles[0].format)) {
        std::cerr << qPrintable(cd.error());
        return 2;
    }
    tr.reportDuplicates(tr.resolveDuplicates(), inFiles[0].name, verbose);

    for (int i = 1; i < inFiles.size(); ++i) {
        Translator tr2;
        if (!tr2.load(inFiles[i].name, cd, inFiles[i].format)) {
            std::cerr << qPrintable(cd.error());
            return 2;
        }
        tr2.reportDuplicates(tr2.resolveDuplicates(), inFiles[i].name, verbose);
        for (int j = 0; j < tr2.messageCount(); ++j)
            tr.replaceSorted(tr2.message(j));

        tr.appendDependencies(tr2.dependencies());
    }

    for (const auto &file: inFiles) {
        tr.satisfyDependency(file.name, file.format);
    }

    if (!targetLanguage.isEmpty())
        tr.setLanguageCode(targetLanguage);
    if (!sourceLanguage.isEmpty())
        tr.setSourceLanguageCode(sourceLanguage);
    if (noObsolete)
        tr.stripObsoleteMessages();
    if (noFinished)
        tr.stripFinishedMessages();
    if (noUntranslated)
        tr.stripUntranslatedMessages();
    if (dropTranslations)
        tr.dropTranslations();
    if (noUiLines)
        tr.dropUiLines();
    if (pluralOnly)
        tr.stripNonPluralForms();
    if (locations != Translator::DefaultLocations)
        tr.setLocationsType(locations);

    tr.normalizeTranslations(cd);
    if (!cd.errors().isEmpty()) {
        std::cerr << qPrintable(cd.error());
        cd.clearErrors();
    }
    if (!tr.save(outFileName, cd, outFormat)) {
        std::cerr << qPrintable(cd.error());
        return 3;
    }
    return 0;
}
