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
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qtranslator.h>

#include <iostream>

QT_USE_NAMESPACE

#ifdef QT_BOOTSTRAPPED
struct LR {
    static inline QString tr(const char *sourceText, const char *comment = 0)
    {
        return QCoreApplication::translate("LRelease", sourceText, comment);
    }
};
#else
class LR {
    Q_DECLARE_TR_FUNCTIONS(LRelease)
};
#endif

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
    printOut(LR::tr(
        "Usage:\n"
        "    lrelease-pro [options] [project-file]...\n"
        "lrelease-pro is part of Qt's Linguist tool chain. It extracts project\n"
        "information from qmake projects and passes it to lrelease.\n"
        "All command line options that are not consumed by lrelease-pro are\n"
        "passed to lrelease.\n\n"
        "Options:\n"
        "    -help  Display this information and exit\n"
        "    -keep  Keep the temporary project dump around\n"
        "    -silent\n"
        "           Do not explain what is being done\n"
        "    -version\n"
        "           Display the version of lrelease-pro and exit\n"
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
#endif // QT_BOOTSTRAPPED

    bool keepProjectDescription = false;
    QStringList inputFiles;
    QStringList lprodumpOptions;
    QStringList lreleaseOptions;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-keep")) {
            keepProjectDescription = true;
        } else if (!strcmp(argv[i], "-silent")) {
            const QString arg = QString::fromLocal8Bit(argv[i]);
            lprodumpOptions << arg;
            lreleaseOptions << arg;
        } else if (!strcmp(argv[i], "-version")) {
            printOut(LR::tr("lrelease-pro version %1\n").arg(QLatin1String(QT_VERSION_STR)));
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
        printErr(LR::tr("lrelease-pro: No .pro/.pri files given.\n"));
        return 1;
    }
    if (!inputFiles.isEmpty()) {
        printErr(LR::tr("lrelease-pro: Only .pro/.pri files are supported. "
                        "Offending files:\n    %1\n")
                 .arg(inputFiles.join(QLatin1String("\n    "))));
        return 1;
    }

    lprodumpOptions << proFiles;
    std::unique_ptr<QTemporaryFile> projectDescription = createProjectDescription(lprodumpOptions);
    if (keepProjectDescription)
        projectDescription->setAutoRemove(false);
    lreleaseOptions << QStringLiteral("-project") << projectDescription->fileName();

    runQtTool(QStringLiteral("lrelease"), lreleaseOptions);
    return 0;
}
