// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <profileutils.h>
#include <projsongenerator.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>

#include <iostream>

using namespace Qt::StringLiterals;

static void printOut(const QString &out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString &out)
{
    std::cerr << qPrintable(out);
}

static void printUsage()
{
    printOut(uR"(Usage:
    lprodump [options] project-file...
lprodump is part of Qt's Linguist tool chain. It extracts information
from qmake projects to a .json file. This file can be passed to
lupdate/lrelease using the -project option.

Options:
    -help  Display this information and exit.
    -silent
           Do not explain what is being done.
    -pro <filename>
           Name of a .pro file. Useful for files with .pro file syntax but
           different file suffix. Projects are recursed into and merged.
    -pro-out <directory>
           Virtual output directory for processing subsequent .pro files.
    -pro-debug
           Trace processing .pro files. Specify twice for more verbosity.
    -out <filename>
           Name of the output file.
    -translations-variables <variable_1>[,<variable_2>,...]
           Comma-separated list of QMake variables containing .ts files.
    -version
           Display the version of lprodump and exit.
)"_s);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    QStringList proFiles;
    QStringList translationsVariables = { u"TRANSLATIONS"_s };
    QString outDir = QDir::currentPath();
    QHash<QString, QString> outDirMap;
    QString outputFilePath;
    int proDebug = 0;
    bool verbose = true;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == "-help"_L1 || arg == "--help"_L1 || arg == "-h"_L1) {
            printUsage();
            return 0;
        } else if (arg == "-out"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -out requires a parameter.\n"_s);
                return 1;
            }
            outputFilePath = args[i];
        } else if (arg == "-silent"_L1) {
            verbose = false;
        } else if (arg == "-pro-debug"_L1) {
            proDebug++;
        } else if (arg == "-version"_L1) {
            printOut(QStringLiteral("lprodump version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == "-pro"_L1) {
            ++i;
            if (i == argc) {
                printErr(QStringLiteral("The -pro option should be followed by a filename of .pro file.\n"));
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            outDirMap[file] = outDir;
        } else if (arg == "-pro-out"_L1) {
            ++i;
            if (i == argc) {
                printErr(QStringLiteral("The -pro-out option should be followed by a directory name.\n"));
                return 1;
            }
            outDir = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
        } else if (arg == u"-translations-variables"_s) {
            ++i;
            if (i == argc) {
                printErr(u"The -translations-variables option must be followed by a "_s
                         u"comma-separated list of variable names.\n"_s);
                return 1;
            }
            translationsVariables = args.at(i).split(u',');
        } else if (arg.startsWith("-"_L1) && arg != "-"_L1) {
            printErr(QStringLiteral("Unrecognized option '%1'.\n").arg(arg));
            return 1;
        } else {
            QFileInfo fi(arg);
            if (!fi.exists()) {
                printErr(QStringLiteral("lprodump error: File '%1' does not exist.\n").arg(arg));
                return 1;
            }
            if (!isProOrPriFile(arg)) {
                printErr(QStringLiteral("lprodump error: '%1' is neither a .pro nor a .pri file.\n")
                         .arg(arg));
                return 1;
            }
            QString cleanFile = QDir::cleanPath(fi.absoluteFilePath());
            proFiles << cleanFile;
            outDirMap[cleanFile] = outDir;
        }
    } // for args

    if (proFiles.isEmpty()) {
        printUsage();
        return 1;
    }

    std::optional<QJsonArray> results = generateProjectDescription(proFiles, translationsVariables,
                                                                   outDirMap, proDebug, verbose);
    if (!results)
        return 1;

    const QByteArray output = QJsonDocument(*results).toJson(QJsonDocument::Compact);
    if (outputFilePath.isEmpty()) {
        puts(output.constData());
    } else {
        QFile f(outputFilePath);
        if (!f.open(QIODevice::WriteOnly)) {
            printErr(QStringLiteral("lprodump error: Cannot open %1 for writing.\n").arg(outputFilePath));
            return 1;
        }
        f.write(output);
        f.write("\n");
    }
    return 0;
}
