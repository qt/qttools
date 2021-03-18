/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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

static void printOut(const QString & out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString & out)
{
    std::cerr << qPrintable(out);
}

class LU {
    Q_DECLARE_TR_FUNCTIONS(LUpdate)
};

static void printUsage()
{
    printOut(LU::tr(
        "Usage:\n"
        "    lupdate-pro [options] [project-file]... [-ts ts-files...]\n"
        "lupdate-pro is part of Qt's Linguist tool chain. It extracts project\n"
        "information from qmake projects and passes it to lupdate.\n"
        "All command line options that are not consumed by lupdate-pro are\n"
        "passed to lupdate.\n\n"
        "Options:\n"
        "    -help  Display this information and exit.\n"
        "    -silent\n"
        "           Do not explain what is being done.\n"
        "    -pro <filename>\n"
        "           Name of a .pro file. Useful for files with .pro file syntax but\n"
        "           different file suffix. Projects are recursed into and merged.\n"
        "    -pro-out <directory>\n"
        "           Virtual output directory for processing subsequent .pro files.\n"
        "    -pro-debug\n"
        "           Trace processing .pro files. Specify twice for more verbosity.\n"
        "    -version\n"
        "           Display the version of lupdate-pro and exit.\n"
    ));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifndef QT_BOOTSTRAPPED
#ifndef Q_OS_WIN32
    QTranslator translator;
    QTranslator qtTranslator;
    QString sysLocale = QLocale::system().name();
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
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
            printOut(LU::tr("lupdate-pro version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == QLatin1String("-pro")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The -pro option should be followed by a filename of .pro file.\n"));
                return 1;
            }
            lprodumpOptions << arg << args[i];
            hasProFiles = true;
        } else if (arg == QLatin1String("-pro-out")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The -pro-out option should be followed by a directory name.\n"));
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
        printErr(LU::tr("lupdate-pro: No .pro/.pri files given.\n"));
        return 1;
    }

    std::unique_ptr<QTemporaryFile> projectDescription = createProjectDescription(lprodumpOptions);
    if (keepProjectDescription)
        projectDescription->setAutoRemove(false);
    lupdateOptions << QStringLiteral("-project") << projectDescription->fileName();

    runQtTool(QStringLiteral("lupdate"), lupdateOptions);
    return 0;
}
