// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <linguistproject/profileutils.h>
#include <linguistproject/projsongenerator.h>
#include <linguistproject/projectdescriptionreader.h>
#include <releasehelper.h>
#include <translator.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qtranslator.h>

#include <iostream>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

static void printUsage()
{
    printOut(uR"(
Usage:
    lrelease-pro [options] [project-file]...
lrelease-pro is part of Qt's Linguist tool chain. It extracts project
information from qmake projects and passes it to lrelease.
All command line options that are not consumed by lrelease-pro are
passed to lrelease.

Options:
    -help  Display this information and exit
    -dump-json <file>
           Only generate the project description json file.
    -fail-on-invalid
           Fail if translations failing the following checks are found:
                validity check of accelerators
                validity check of surrounding whitespaces
                validity check of ending punctuation
                validity check of place markers
           To get more details refer to Qt Linguist help
    -silent
           Do not explain what is being done
    -version
           Display the version of lrelease-pro and exit
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
#endif // QT_BOOTSTRAPPED

    std::optional<QString> dumpJsonFile;
    QStringList inputFiles;

    ParamFlags params;
    QString markUntranslated;
    ConversionData cd;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "-silent"_L1) {
            cd.m_verbose = false;
        } else if (arg == "-verbose"_L1) {
            cd.m_verbose = true;
        } else if (arg == "-version"_L1) {
            printOut(QStringLiteral("lrelease-pro version %1\n")
                     .arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == "-help"_L1) {
            printUsage();
            return 0;
        } else if (arg == "-dump-json"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -dump-json option should be followed by a file name.\n"_s);
                return 1;
            }
            dumpJsonFile.emplace(QFileInfo(QLatin1String(argv[i])).absoluteFilePath());
        } else if (arg == "-removeidentical"_L1) {
            params.removeIdentical = true;
        } else if (arg == "-fail-on-unfinished"_L1) {
            params.failOnUnfinished = true;
        } else if (arg ==  "-fail-on-invalid"_L1) {
            params.failOnInvalid = true;
        } else if (arg == "-nounfinished"_L1) {
            cd.m_ignoreUnfinished = true;
        } else if (arg == "-markuntranslated"_L1) {
            ++i;
            if (i < argc)
                markUntranslated = QString::fromLocal8Bit(argv[i]);
        } else if (arg == "-idbased"_L1) {
            printOut("The flag -idbased is deprecated and not required anymore."
                     "It will be removed in a future version\n"_L1);
        } else if (!arg.startsWith(u'-')) {
            inputFiles << arg;
        }
    }

    if (inputFiles.isEmpty()) {
        printUsage();
        return 1;
    }

    const QStringList proFiles = extractProFiles(&inputFiles);
    if (proFiles.isEmpty()) {
        printErr(u"lrelease-pro: No .pro/.pri files given.\n"_s);
        return 1;
    }
    if (!inputFiles.isEmpty()) {
        printErr(QStringLiteral("lrelease-pro: Only .pro/.pri files are supported. "
                                "Offending files:\n    %1\n")
                         .arg(inputFiles.join("\n    "_L1)));
        return 1;
    }

    QStringList translationsVariables = { "TRANSLATIONS"_L1, "EXTRA_TRANSLATIONS"_L1 };
    QString outDir = QDir::currentPath();
    QHash<QString, QString> outDirMap;

    QStringList cleanProFiles;
    for (const QString &proFile : proFiles) {
        QString cleanFile = QDir::cleanPath(QFileInfo(proFile).absoluteFilePath());
        cleanProFiles << cleanFile;
        outDirMap[cleanFile] = outDir;
    }

    QString errorString;
    QJsonArray results;
    Projects projects = generateProjects(cleanProFiles, translationsVariables, outDirMap, 0,
                                         cd.m_verbose, &errorString, &results);

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
        printErr(QStringLiteral("lrelease-pro error: %1\n").arg(errorString));
        return 1;
    }
    if (projects.empty()) {
        printErr(u"lrelease-pro error: No projects found in project description\n"_s);
        return 1;
    }

    QStringList tsFiles = translationsFromProjects(projects);
    if (tsFiles.isEmpty()) {
        printErr(u"lrelease-pro error: no TS files specified.\n"_s);
        return 1;
    }

    if (!markUntranslated.isEmpty())
        cd.m_unTrPrefix = markUntranslated;

    bool fail = false;
    for (const QString &tsFile : std::as_const(tsFiles)) {
        if (!releaseTsFile(tsFile, cd, params))
            fail = true;
    }

    return fail ? 1 : 0;
}
