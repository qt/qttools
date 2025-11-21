// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <profileutils.h>
#include <projsongenerator.h>
#include <runqttool.h>

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
    printOut(
        uR"(Usage:
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
    QStringList lupdateOptions;
    bool keepProjectDescription = false;

    QStringList proFiles;
    QString outDir = QDir::currentPath();
    QHash<QString, QString> outDirMap;
    int proDebug = 0;
    bool verbose = true;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == "-help"_L1 || arg == "--help"_L1 || arg == "-h"_L1) {
            printUsage();
            return 0;
        } else if (arg == "-keep"_L1) {
            keepProjectDescription = true;
        } else if (arg == "-silent"_L1) {
            lupdateOptions << arg;
            verbose = false;
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
        } else if (isProOrPriFile(arg)) {
            QString cleanFile = QDir::cleanPath(QFileInfo(arg).absoluteFilePath());
            proFiles << cleanFile;
            outDirMap[cleanFile] = outDir;
        } else {
            lupdateOptions << arg;
        }
    } // for args

    if (proFiles.isEmpty()) {
        printErr(u"lupdate-pro: No .pro/.pri files given.\n"_s);
        return 1;
    }

    bool ok = false;
    QStringList translationsVariables = { "TRANSLATIONS"_L1 };
    QJsonArray results = generateProjectDescription(proFiles, translationsVariables, outDirMap,
                                                    proDebug, verbose, &ok);
    if (!ok) {
        printErr(u"lupdate-pro: Failed to generate project description\n"_s);
        return 1;
    }

    std::unique_ptr<QTemporaryFile> projectDescription(
            new QTemporaryFile(QStringLiteral("XXXXXX.json")));
    if (!projectDescription->open()) {
        printErr(u"lupdate-pro: Cannot create temporary file\n"_s);
        return 1;
    }

    const QByteArray output = QJsonDocument(results).toJson(QJsonDocument::Compact);
    projectDescription->write(output);
    projectDescription->write("\n");
    projectDescription->close();

    if (keepProjectDescription)
        projectDescription->setAutoRemove(false);
    lupdateOptions << QStringLiteral("-project") << projectDescription->fileName();

    runQtTool(QStringLiteral("lupdate"), lupdateOptions);
    return 0;
}
