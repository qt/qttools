/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "translator.h"

#include <profileutils.h>
#include <projectdescriptionreader.h>
#include <runqttool.h>

#ifndef QT_BOOTSTRAPPED
#include <QtCore/QCoreApplication>
#include <QtCore/QTranslator>
#endif
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QLibraryInfo>

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

static void printOut(const QString & out)
{
    QTextStream stream(stdout);
    stream << out;
}

static void printErr(const QString & out)
{
    QTextStream stream(stderr);
    stream << out;
}

static void printUsage()
{
    printOut(LR::tr(
        "Usage:\n"
        "    lrelease [options] -project project-file\n"
        "    lrelease [options] ts-files [-qm qm-file]\n\n"
        "lrelease is part of Qt's Linguist tool chain. It can be used as a\n"
        "stand-alone tool to convert XML-based translations files in the TS\n"
        "format into the 'compiled' QM format used by QTranslator objects.\n\n"
        "Passing .pro files to lrelease is deprecated.\n"
        "Please use the lrelease-pro tool instead, or use qmake's lrelease.prf\n"
        "feature.\n\n"
        "Options:\n"
        "    -help  Display this information and exit\n"
        "    -idbased\n"
        "           Use IDs instead of source strings for message keying\n"
        "    -compress\n"
        "           Compress the QM files\n"
        "    -nounfinished\n"
        "           Do not include unfinished translations\n"
        "    -removeidentical\n"
        "           If the translated text is the same as\n"
        "           the source text, do not include the message\n"
        "    -markuntranslated <prefix>\n"
        "           If a message has no real translation, use the source text\n"
        "           prefixed with the given string instead\n"
        "    -project <filename>\n"
        "           Name of a file containing the project's description in JSON format.\n"
        "           Such a file may be generated from a .pro file using the lprodump tool.\n"
        "    -silent\n"
        "           Do not explain what is being done\n"
        "    -version\n"
        "           Display the version of lrelease and exit\n"
    ));
}

static bool loadTsFile(Translator &tor, const QString &tsFileName, bool /* verbose */)
{
    ConversionData cd;
    bool ok = tor.load(tsFileName, cd, QLatin1String("auto"));
    if (!ok) {
        printErr(LR::tr("lrelease error: %1").arg(cd.error()));
    } else {
        if (!cd.errors().isEmpty())
            printOut(cd.error());
    }
    cd.clearErrors();
    return ok;
}

static bool releaseTranslator(Translator &tor, const QString &qmFileName,
    ConversionData &cd, bool removeIdentical)
{
    tor.reportDuplicates(tor.resolveDuplicates(), qmFileName, cd.isVerbose());

    if (cd.isVerbose())
        printOut(LR::tr("Updating '%1'...\n").arg(qmFileName));
    if (removeIdentical) {
        if (cd.isVerbose())
            printOut(LR::tr("Removing translations equal to source text in '%1'...\n").arg(qmFileName));
        tor.stripIdenticalSourceTranslations();
    }

    QFile file(qmFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        printErr(LR::tr("lrelease error: cannot create '%1': %2\n")
                                .arg(qmFileName, file.errorString()));
        return false;
    }

    tor.normalizeTranslations(cd);
    bool ok = saveQM(tor, file, cd);
    file.close();

    if (!ok) {
        printErr(LR::tr("lrelease error: cannot save '%1': %2")
                                .arg(qmFileName, cd.error()));
    } else if (!cd.errors().isEmpty()) {
        printOut(cd.error());
    }
    cd.clearErrors();
    return ok;
}

static bool releaseTsFile(const QString& tsFileName,
    ConversionData &cd, bool removeIdentical)
{
    Translator tor;
    if (!loadTsFile(tor, tsFileName, cd.isVerbose()))
        return false;

    QString qmFileName = tsFileName;
    foreach (const Translator::FileFormat &fmt, Translator::registeredFileFormats()) {
        if (qmFileName.endsWith(QLatin1Char('.') + fmt.extension)) {
            qmFileName.chop(fmt.extension.length() + 1);
            break;
        }
    }
    qmFileName += QLatin1String(".qm");

    return releaseTranslator(tor, qmFileName, cd, removeIdentical);
}

static QStringList translationsFromProjects(const Projects &projects, bool topLevel);

static QStringList translationsFromProject(const Project &project, bool topLevel)
{
    QStringList result;
    if (project.translations)
        result = *project.translations;
    result << translationsFromProjects(project.subProjects, false);
    if (topLevel && result.isEmpty()) {
        printErr(LR::tr("lrelease warning: Met no 'TRANSLATIONS' entry in project file '%1'\n")
                 .arg(project.filePath));
    }
    return result;
}

static QStringList translationsFromProjects(const Projects &projects, bool topLevel = true)
{
    QStringList result;
    for (const Project &p : projects)
        result << translationsFromProject(p, topLevel);
    return result;
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

    ConversionData cd;
    cd.m_verbose = true; // the default is true starting with Qt 4.2
    bool removeIdentical = false;
    Translator tor;
    QStringList inputFiles;
    QString outputFile;
    QString projectDescriptionFile;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-compress")) {
            cd.m_saveMode = SaveStripped;
            continue;
        } else if (!strcmp(argv[i], "-idbased")) {
            cd.m_idBased = true;
            continue;
        } else if (!strcmp(argv[i], "-nocompress")) {
            cd.m_saveMode = SaveEverything;
            continue;
        } else if (!strcmp(argv[i], "-removeidentical")) {
            removeIdentical = true;
            continue;
        } else if (!strcmp(argv[i], "-nounfinished")) {
            cd.m_ignoreUnfinished = true;
            continue;
        } else if (!strcmp(argv[i], "-markuntranslated")) {
            if (i == argc - 1) {
                printUsage();
                return 1;
            }
            cd.m_unTrPrefix = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(argv[i], "-project")) {
            if (i == argc - 1) {
                printErr(LR::tr("The option -project requires a parameter.\n"));
                return 1;
            }
            if (!projectDescriptionFile.isEmpty()) {
                printErr(LR::tr("The option -project must appear only once.\n"));
                return 1;
            }
            projectDescriptionFile = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(argv[i], "-silent")) {
            cd.m_verbose = false;
            continue;
        } else if (!strcmp(argv[i], "-verbose")) {
            cd.m_verbose = true;
            continue;
        } else if (!strcmp(argv[i], "-version")) {
            printOut(LR::tr("lrelease version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (!strcmp(argv[i], "-qm")) {
            if (i == argc - 1) {
                printUsage();
                return 1;
            }
            outputFile = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(argv[i], "-help")) {
            printUsage();
            return 0;
        } else if (argv[i][0] == '-') {
            printUsage();
            return 1;
        } else {
            inputFiles << QString::fromLocal8Bit(argv[i]);
        }
    }

    if (inputFiles.isEmpty() && projectDescriptionFile.isEmpty()) {
        printUsage();
        return 1;
    }

    QString errorString;
    if (!extractProFiles(&inputFiles).isEmpty()) {
        runQtTool(QStringLiteral("lrelease-pro"), app.arguments().mid(1));
        return 0;
    }

    if (!projectDescriptionFile.isEmpty()) {
        if (!inputFiles.isEmpty()) {
            printErr(LR::tr("lrelease error: Do not specify TS files if -project is given.\n"));
            return 1;
        }
        Projects projectDescription = readProjectDescription(projectDescriptionFile, &errorString);
        if (!errorString.isEmpty()) {
            printErr(LR::tr("lrelease error: %1\n").arg(errorString));
            return 1;
        }
        inputFiles = translationsFromProjects(projectDescription);
    }

    foreach (const QString &inputFile, inputFiles) {
        if (outputFile.isEmpty()) {
            if (!releaseTsFile(inputFile, cd, removeIdentical))
                return 1;
        } else {
            if (!loadTsFile(tor, inputFile, cd.isVerbose()))
                return 1;
        }
    }

    if (!outputFile.isEmpty())
        return releaseTranslator(tor, outputFile, cd, removeIdentical) ? 0 : 1;

    return 0;
}
