// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <linguistproject/profileutils.h>
#include <linguistproject/projsongenerator.h>
#include <linguistproject/projectdescriptionreader.h>
#include <linguistproject/projectprocessor.h>
#include <trlib/trparser.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qtranslator.h>

#include <iostream>

using namespace Qt::StringLiterals;

static void printOut(const QString & out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString & out)
{
    std::cerr << qPrintable(out);
}

static void printUsage()
{
    printOut(uR"(Usage:
lupdate-pro [options] [project-file]... [-ts ts-files...]
lupdate-pro is part of Qt's Linguist tool chain. It extracts project
information from qmake projects and passes it to lupdate.
All command line options that are not consumed by lupdate-pro are
passed to lupdate.

Options:
    -help  Display this information and exit.
    -silent
           Do not explain what is being done.
    -pro <filename>
           Name of a .pro file. Useful for files with .pro file syntax but
           different file suffix. Projects are recursed into and merged.
    -dump-json <file>
           Only generate the project description json file.
    -pro-out <directory>
           Virtual output directory for processing subsequent .pro files.
    -pro-debug
           Trace processing .pro files. Specify twice for more verbosity.
    -version
           Display the version of lupdate-pro and exit.
)"_s);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifndef QT_BOOTSTRAPPED
#ifndef Q_OS_WIN32
    QTranslator translator;
    QTranslator qtTranslator;
    QString sysLocale = QLocale::system().name();
    QString resourceDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (translator.load("linguist_"_L1 + sysLocale, resourceDir)
        && qtTranslator.load("qt_"_L1 + sysLocale, resourceDir)) {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
    }
#endif // Q_OS_WIN32
#endif

    QStringList args = app.arguments();
    std::optional<QString> dumpJsonFile;

    QStringList proFiles;
    QString outDir = QDir::currentPath();
    QHash<QString, QString> outDirMap;
    int proDebug = 0;
    bool verbose = true;

    QStringList tsFileNames;
    QStringList alienFiles;
    QString sourceLanguage;
    QString targetLanguage;
    UpdateOptions options = Verbose | // verbose is on by default starting with Qt 4.2
            HeuristicSameText | HeuristicSimilarText;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == "-help"_L1 || arg == "--help"_L1 || arg == "-h"_L1) {
            printUsage();
            return 0;
        } else if (arg == "-dump-json"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -dump-json option should be followed by a file name.\n"_s);
                return 1;
            }
            dumpJsonFile.emplace(QFileInfo(args[i]).absoluteFilePath());
        } else if (arg == "-silent"_L1) {
            verbose = false;
            options &= ~Verbose;
        } else if (arg == "-verbose"_L1) {
            options |= Verbose;
        } else if (arg == "-pro-debug"_L1) {
            proDebug++;
        } else if (arg == "-version"_L1) {
            printOut(QStringLiteral("lupdate-pro version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == "-pro"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -pro option should be followed by a filename of .pro file.\n"_s);
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            outDirMap[file] = outDir;
        } else if (arg == "-pro-out"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -pro-out option should be followed by a directory name.\n"_s);
                return 1;
            }
            outDir = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
        } else if (arg == "-ts"_L1) {
            ++i;
            while (i < args.size() && !args.at(i).startsWith('-'_L1)) {
                tsFileNames << args.at(i);
                ++i;
            }
            --i;
        } else if (arg == "-target-language"_L1) {
            ++i;
            if (i < args.size())
                targetLanguage = args.at(i);
        } else if (arg == "-source-language"_L1) {
            ++i;
            if (i < args.size())
                sourceLanguage = args.at(i);
        } else if (arg == "-no-obsolete"_L1 || arg == "-noobsolete"_L1) {
            options |= NoObsolete;
        } else if (arg == "-plural-only"_L1) {
            options |= PluralOnly;
        } else if (arg == "-no-sort"_L1 || arg == "-nosort"_L1) {
            options |= NoSort;
        } else if (arg == "-locations"_L1) {
            ++i;
            if (i < args.size()) {
                if (args.at(i) == "none"_L1)
                    options |= NoLocations;
                else if (args.at(i) == "relative"_L1)
                    options |= RelativeLocations;
                else if (args.at(i) == "absolute"_L1)
                    options |= AbsoluteLocations;
            }
        } else if (arg == "-no-ui-lines"_L1) {
            options |= NoUiLines;
        } else if (arg == "-tr-function-alias"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -tr-function-alias option should be "
                         "followed by a list of function=alias mappings.\n"_s);
                return 1;
            }
            if (!parseTrFunctionAliasString(args[i]))
                return 1;
        } else if (isProOrPriFile(arg)) {
            QString cleanFile = QDir::cleanPath(QFileInfo(arg).absoluteFilePath());
            proFiles << cleanFile;
            outDirMap[cleanFile] = outDir;
        }
    } // for args

    if (proFiles.isEmpty()) {
        printErr(u"lupdate-pro: No .pro/.pri files given.\n"_s);
        return 1;
    }

    QStringList translationsVariables = { "TRANSLATIONS"_L1 };
    QString errorString;
    QJsonArray results;
    Projects projectDescription = generateProjects(proFiles, translationsVariables, outDirMap,
                                                   proDebug, verbose, &errorString, &results);

    if (dumpJsonFile) {
        QFile projectDescriptionFile(*dumpJsonFile);
        if (!projectDescriptionFile.open(QIODevice::WriteOnly))
            return 1;

        const QByteArray output = QJsonDocument(results).toJson(QJsonDocument::Compact);
        projectDescriptionFile.write(output);
        projectDescriptionFile.write("\n");
        projectDescriptionFile.close();
        printOut("Project description saved to: %1\n"_L1.arg(projectDescriptionFile.fileName()));
        return 0;
    }

    if (!errorString.isEmpty()) {
        printErr("lupdate-pro error: %1\n"_L1.arg(errorString));
        return 1;
    }
    if (projectDescription.empty()) {
        printErr("lupdate-pro error: No projects found in project description\n"_L1);
        return 1;
    }

    bool ok = processProjectDescription(projectDescription, tsFileNames, alienFiles, sourceLanguage,
                              targetLanguage, options);
    return ok ? 0 : 1;
}
