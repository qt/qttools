// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <profileutils.h>
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
    if (translator.load(QLatin1String("linguist_") + sysLocale, resourceDir)
        && qtTranslator.load(QLatin1String("qt_") + sysLocale, resourceDir)) {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
    }
#endif // Q_OS_WIN32
#endif

    QStringList args = app.arguments();
    QStringList lupdateOptions;
    QStringList lprodumpOptions;
    bool hasProFiles = false;
    bool keepProjectDescription = false;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == QLatin1String("-help")
                || arg == QLatin1String("--help")
                || arg == QLatin1String("-h")) {
            printUsage();
            return 0;
        } else if (arg == QLatin1String("-keep")) {
            keepProjectDescription = true;
        } else if (arg == QLatin1String("-silent")) {
            lupdateOptions << arg;
            lprodumpOptions << arg;
        } else if (arg == QLatin1String("-pro-debug")) {
            lprodumpOptions << arg;
        } else if (arg == QLatin1String("-version")) {
            printOut(QStringLiteral("lupdate-pro version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == QLatin1String("-pro")) {
            ++i;
            if (i == argc) {
                printErr(u"The -pro option should be followed by a filename of .pro file.\n"_s);
                return 1;
            }
            lprodumpOptions << arg << args[i];
            hasProFiles = true;
        } else if (arg == QLatin1String("-pro-out")) {
            ++i;
            if (i == argc) {
                printErr(u"The -pro-out option should be followed by a directory name.\n"_s);
                return 1;
            }
            lprodumpOptions << arg << args[i];
        } else if (isProOrPriFile(arg)) {
            lprodumpOptions << arg;
            hasProFiles = true;
        } else {
            lupdateOptions << arg;
        }
    } // for args

    if (!hasProFiles) {
        printErr(u"lupdate-pro: No .pro/.pri files given.\n"_s);
        return 1;
    }

    std::unique_ptr<QTemporaryFile> projectDescription = createProjectDescription(lprodumpOptions);
    if (keepProjectDescription)
        projectDescription->setAutoRemove(false);
    lupdateOptions << QStringLiteral("-project") << projectDescription->fileName();

    runQtTool(QStringLiteral("lupdate"), lupdateOptions);
    return 0;
}
