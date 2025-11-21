// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <profileutils.h>
#include <projsongenerator.h>
#include <runqttool.h>

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
    printOut(uR"(
Usage:
    lrelease-pro [options] [project-file]...
lrelease-pro is part of Qt's Linguist tool chain. It extracts project
information from qmake projects and passes it to lrelease.
All command line options that are not consumed by lrelease-pro are
passed to lrelease.

Options:
    -help  Display this information and exit
    -keep  Keep the temporary project dump around
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

    bool keepProjectDescription = false;
    QStringList inputFiles;
    QStringList lreleaseOptions;

    bool verbose = true;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-keep")) {
            keepProjectDescription = true;
        } else if (!strcmp(argv[i], "-silent")) {
            lreleaseOptions << QString::fromLocal8Bit(argv[i]);
            verbose = false;
        } else if (!strcmp(argv[i], "-version")) {
            printOut(QStringLiteral("lrelease-pro version %1\n")
                     .arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (!strcmp(argv[i], "-help")) {
            printUsage();
            return 0;
        } else if (strlen(argv[i]) > 0 && argv[i][0] == '-') {
            lreleaseOptions << QString::fromLocal8Bit(argv[i]);
        } else {
            inputFiles << QString::fromLocal8Bit(argv[i]);
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

    std::optional<QJsonArray> results =
            generateProjectDescription(cleanProFiles, translationsVariables, outDirMap, 0, verbose);
    if (!results) {
        printErr(u"lrelease-pro: Failed to generate project description\n"_s);
        return 1;
    }

    std::unique_ptr<QTemporaryFile> projectDescription(
            new QTemporaryFile(QStringLiteral("XXXXXX.json")));
    if (!projectDescription->open()) {
        printErr(u"lrelease-pro: Cannot create temporary file\n"_s);
        return 1;
    }

    const QByteArray output = QJsonDocument(*results).toJson(QJsonDocument::Compact);
    projectDescription->write(output);
    projectDescription->write("\n");
    projectDescription->close();

    if (keepProjectDescription)
        projectDescription->setAutoRemove(false);
    lreleaseOptions << QStringLiteral("-project") << projectDescription->fileName();

    runQtTool(QStringLiteral("lrelease"), lreleaseOptions);
    return 0;
}
