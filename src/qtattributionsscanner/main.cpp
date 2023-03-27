// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "jsongenerator.h"
#include "logging.h"
#include "packagefilter.h"
#include "qdocgenerator.h"
#include "scanner.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName(u"Qt Attributions Scanner"_s);
    a.setApplicationVersion(u"1.2"_s);

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("Processes attribution files in Qt sources."));
    parser.addPositionalArgument(u"path"_s,
                                 tr("Path to a qt_attribution.json/README.chromium file, "
                                    "or a directory to be scannned recursively."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption generatorOption(u"output-format"_s,
                                       tr("Output format (\"qdoc\", \"json\")."),
                                       u"generator"_s, u"qdoc"_s);
    QCommandLineOption inputFormatOption(u"input-files"_s,
                                       tr("Input files (\"qt_attributions\" scans for "
                                          "qt_attribution.json, \"chromium_attributions\" for "
                                          "README.Chromium, \"all\" for both)."),
                                       u"input_format"_s,
                                       u"qt_attributions"_s);
    QCommandLineOption filterOption(u"filter"_s,
                                    tr("Filter packages according to <filter> "
                                       "(e.g. QDocModule=qtcore)"),
                                    u"expression"_s);
    QCommandLineOption baseDirOption(u"basedir"_s,
                                     tr("Paths in documentation are made relative to this "
                                        "directory."),
                                     u"directory"_s);
    QCommandLineOption noCheckPathsOption(
            u"no-check-paths"_s,
            tr("Do not check whether referenced file paths exist in basedir."));
    QCommandLineOption outputOption({ u"o"_s, u"output"_s },
                                    tr("Write generated data to <file>."),
                                    u"file"_s);
    QCommandLineOption verboseOption(u"verbose"_s, tr("Verbose output."));
    QCommandLineOption silentOption({ u"s"_s, u"silent"_s }, tr("Minimal output."));

    parser.addOption(generatorOption);
    parser.addOption(inputFormatOption);
    parser.addOption(filterOption);
    parser.addOption(baseDirOption);
    parser.addOption(noCheckPathsOption);
    parser.addOption(outputOption);
    parser.addOption(verboseOption);
    parser.addOption(silentOption);

    parser.process(a.arguments());

    using Scanner::Checks, Scanner::Check;
    Checks checks = Check::All;
    checks.setFlag(Check::Paths, !parser.isSet(noCheckPathsOption));

    LogLevel logLevel = NormalLog;
    if (parser.isSet(verboseOption) && parser.isSet(silentOption)) {
        std::cerr << qPrintable(tr("--verbose and --silent cannot be set simultaneously.")) << std::endl;
        parser.showHelp(1);
    }

    if (parser.isSet(verboseOption))
        logLevel = VerboseLog;
    else if (parser.isSet(silentOption))
        logLevel = SilentLog;

    if (parser.positionalArguments().size() != 1)
        parser.showHelp(2);

    const QString path = parser.positionalArguments().constLast();

    QString inputFormat = parser.value(inputFormatOption);
    Scanner::InputFormats formats;
    if (inputFormat == "qt_attributions"_L1)
        formats = Scanner::InputFormat::QtAttributions;
    else if (inputFormat == "chromium_attributions"_L1)
        formats = Scanner::InputFormat::ChromiumAttributions;
    else if (inputFormat == "all"_L1)
        formats = Scanner::InputFormat::QtAttributions | Scanner::InputFormat::ChromiumAttributions;
    else {
        std::cerr << qPrintable(tr("%1 is not a valid input-files argument").arg(inputFormat)) << std::endl << std::endl;
        parser.showHelp(8);
    }

    // Parse the attribution files
    QList<Package> packages;
    const QFileInfo pathInfo(path);
    if (pathInfo.isDir()) {
        if (logLevel == VerboseLog)
            std::cerr << qPrintable(tr("Recursively scanning %1 for attribution files...").arg(
                                        QDir::toNativeSeparators(path))) << std::endl;
        std::optional<QList<Package>> p = Scanner::scanDirectory(path, formats, checks, logLevel);
        if (!p)
            return 1;
        packages = *p;
    } else if (pathInfo.isFile()) {
        std::optional<QList<Package>> p = Scanner::readFile(path, checks, logLevel);
        if (!p)
            return 1;
        packages = *p;

    } else {
        std::cerr << qPrintable(tr("%1 is not a valid file or directory.").arg(
                                    QDir::toNativeSeparators(path))) << std::endl << std::endl;
        parser.showHelp(7);
    }

    // Apply the filter
    if (parser.isSet(filterOption)) {
        PackageFilter filter(parser.value(filterOption));
        if (filter.type == PackageFilter::InvalidFilter)
            return 4;
        packages.erase(std::remove_if(packages.begin(), packages.end(),
                                      [&filter](const Package &p) { return !filter(p); }),
                       packages.end());
    }

    if (logLevel == VerboseLog)
        std::cerr << qPrintable(tr("%1 packages found.").arg(packages.size())) << std::endl;

    // Prepare the output text stream
    QFile outFile(parser.value(outputOption));
    QTextStream out(stdout);
    if (!outFile.fileName().isEmpty()) {
        if (!outFile.open(QFile::WriteOnly)) {
            std::cerr << qPrintable(tr("Cannot open %1 for writing.").arg(
                                        QDir::toNativeSeparators(outFile.fileName())))
                      << std::endl;
            return 5;
        }
        out.setDevice(&outFile);
    }

    // Generate the output and write it
    QString generator = parser.value(generatorOption);
    if (generator == "qdoc"_L1) {
        QString baseDirectory = parser.value(baseDirOption);
        if (baseDirectory.isEmpty()) {
            if (pathInfo.isDir()) {
                // include top level module name in printed paths
                baseDirectory = pathInfo.dir().absoluteFilePath(u".."_s);
            } else {
                baseDirectory = pathInfo.absoluteDir().absoluteFilePath(u".."_s);
            }
        }

        QDocGenerator::generate(out, packages, baseDirectory, logLevel);
    } else if (generator == "json"_L1) {
        JsonGenerator::generate(out, packages, logLevel);
    } else {
        std::cerr << qPrintable(tr("Unknown output-format %1.").arg(generator)) << std::endl;
        return 6;
    }

    if (logLevel == VerboseLog)
        std::cerr << qPrintable(tr("Processing is done.")) << std::endl;

    return 0;
}
