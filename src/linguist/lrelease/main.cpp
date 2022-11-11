// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QLibraryInfo>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

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
    printOut(uR"(Usage:
    lrelease [options] -project project-file
    lrelease [options] ts-files [-qm qm-file]

lrelease is part of Qt's Linguist tool chain. It can be used as a
stand-alone tool to convert XML-based translations files in the TS
format into the 'compiled' QM format used by QTranslator objects.

Passing .pro files to lrelease is deprecated.
Please use the lrelease-pro tool instead, or use qmake's lrelease.prf
feature.

Options:
    -help  Display this information and exit
    -idbased
           Use IDs instead of source strings for message keying
    -compress
           Compress the QM files
    -nounfinished
           Do not include unfinished translations
    -removeidentical
           If the translated text is the same as
           the source text, do not include the message
    -markuntranslated <prefix>
           If a message has no real translation, use the source text
           prefixed with the given string instead
    -project <filename>
           Name of a file containing the project's description in JSON format.
           Such a file may be generated from a .pro file using the lprodump tool.
    -silent
           Do not explain what is being done
    -version
           Display the version of lrelease and exit
)"_s);
}

static bool loadTsFile(Translator &tor, const QString &tsFileName, bool /* verbose */)
{
    ConversionData cd;
    bool ok = tor.load(tsFileName, cd, QLatin1String("auto"));
    if (!ok) {
        printErr(QLatin1String("lrelease error: %1").arg(cd.error()));
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
        printOut(QLatin1String("Updating '%1'...\n").arg(qmFileName));
    if (removeIdentical) {
        if (cd.isVerbose())
            printOut(QLatin1String("Removing translations equal to source text in '%1'...\n")
                             .arg(qmFileName));
        tor.stripIdenticalSourceTranslations();
    }

    QFile file(qmFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        printErr(QLatin1String("lrelease error: cannot create '%1': %2\n")
                         .arg(qmFileName, file.errorString()));
        return false;
    }

    tor.normalizeTranslations(cd);
    bool ok = saveQM(tor, file, cd);
    file.close();

    if (!ok) {
        printErr(QLatin1String("lrelease error: cannot save '%1': %2").arg(qmFileName, cd.error()));
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
    for (const Translator::FileFormat &fmt : std::as_const(Translator::registeredFileFormats())) {
        if (qmFileName.endsWith(QLatin1Char('.') + fmt.extension)) {
            qmFileName.chop(fmt.extension.size() + 1);
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
        printErr(
            QLatin1String("lrelease warning: Met no 'TRANSLATIONS' entry in project file '%1'\n")
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
                printErr(QLatin1String("The option -project requires a parameter.\n"));
                return 1;
            }
            if (!projectDescriptionFile.isEmpty()) {
                printErr(QLatin1String("The option -project must appear only once.\n"));
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
            printOut(QLatin1String("lrelease version %1\n").arg(QLatin1String(QT_VERSION_STR)));
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
        runInternalQtTool(QLatin1String("lrelease-pro"), app.arguments().mid(1));
        return 0;
    }

    if (!projectDescriptionFile.isEmpty()) {
        if (!inputFiles.isEmpty()) {
            printErr(QLatin1String(
                    "lrelease error: Do not specify TS files if -project is given.\n"));
            return 1;
        }
        Projects projectDescription = readProjectDescription(projectDescriptionFile, &errorString);
        if (!errorString.isEmpty()) {
            printErr(QLatin1String("lrelease error: %1\n").arg(errorString));
            return 1;
        }
        inputFiles = translationsFromProjects(projectDescription);
    }

    for (const QString &inputFile : std::as_const(inputFiles)) {
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
