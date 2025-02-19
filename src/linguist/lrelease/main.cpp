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
           Deprecated. The flag is not required anymore and will be removed
           in a future version. It was used to enable ID based translation.
    -compress
           Compress the QM files
    -nounfinished
           Do not include unfinished translations
    -fail-on-unfinished
            Generate an error if unfinished translations are found
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
    -verbose
           Explain what is being done (default)
    -version
           Display the version of lrelease and exit
)"_s);
}

static bool loadTsFile(Translator &tor, const QString &tsFileName)
{
    ConversionData cd;
    bool ok = tor.load(tsFileName, cd, "auto"_L1);
    if (!ok) {
        printErr("lrelease error: %1"_L1.arg(cd.error()));
    } else {
        if (!cd.errors().isEmpty())
            printOut(cd.error());
    }
    cd.clearErrors();
    return ok;
}

static bool releaseTranslator(Translator &tor, const QString &qmFileName, ConversionData &cd,
                              bool removeIdentical, bool failOnUnfinished)
{
    if (failOnUnfinished && tor.unfinishedTranslationsExist()) {
        printErr("lrelease error: cannot create '%1': existing unfinished translation(s) "
                 "found (-fail-on-unfinished)"_L1.arg(qmFileName));
        return false;
    }

    tor.reportDuplicates(tor.resolveDuplicates(), qmFileName, cd.isVerbose());

    if (cd.isVerbose())
        printOut("Updating '%1'...\n"_L1.arg(qmFileName));
    if (removeIdentical) {
        if (cd.isVerbose())
            printOut("Removing translations equal to source text in '%1'...\n"_L1.arg(qmFileName));
        tor.stripIdenticalSourceTranslations();
    }

    QFile file(qmFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        printErr("lrelease error: cannot create '%1': %2\n"_L1.arg(qmFileName, file.errorString()));
        return false;
    }

    tor.normalizeTranslations(cd);
    bool ok = saveQM(tor, file, cd);
    file.close();

    if (!ok) {
        printErr("lrelease error: cannot save '%1': %2"_L1.arg(qmFileName, cd.error()));
    } else if (!cd.errors().isEmpty()) {
        printOut(cd.error());
    }
    cd.clearErrors();
    return ok;
}

static bool releaseTsFile(const QString &tsFileName, ConversionData &cd, bool removeIdentical,
                          bool failOnUnfinished)
{
    Translator tor;
    if (!loadTsFile(tor, tsFileName))
        return false;

    QString qmFileName = tsFileName;
    for (const Translator::FileFormat &fmt : std::as_const(Translator::registeredFileFormats())) {
        if (qmFileName.endsWith(u'.' + fmt.extension)) {
            qmFileName.chop(fmt.extension.size() + 1);
            break;
        }
    }
    qmFileName += ".qm"_L1;

    return releaseTranslator(tor, qmFileName, cd, removeIdentical, failOnUnfinished);
}

static QStringList translationsFromProjects(const Projects &projects, bool topLevel);

static QStringList translationsFromProject(const Project &project, bool topLevel)
{
    QStringList result;
    if (project.translations)
        result = *project.translations;
    result << translationsFromProjects(project.subProjects, false);
    if (topLevel && result.isEmpty()) {
        printErr("lrelease warning: Met no 'TRANSLATIONS' entry in project file '%1'\n"_L1.arg(
                project.filePath));
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
    bool failOnUnfinished = false;
    Translator tor;
    QStringList inputFiles;
    QString outputFile;
    QString projectDescriptionFile;

    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "-compress")) {
            cd.m_saveMode = SaveStripped;
            continue;
        } else if (!strcmp(arg, "-idbased")) {
            printOut("The flag -idbased is depreciated and not required anymore."
                     "It will be removed in a future version"_L1);
            continue;
        } else if (!strcmp(arg, "-nocompress")) {
            cd.m_saveMode = SaveEverything;
            continue;
        } else if (!strcmp(arg, "-removeidentical")) {
            removeIdentical = true;
            continue;
        } else if (!strcmp(arg, "-nounfinished")) {
            cd.m_ignoreUnfinished = true;
            continue;
        } else if (!strcmp(arg, "-fail-on-unfinished")) {
            failOnUnfinished = true;
            continue;
        } else if (!strcmp(arg, "-markuntranslated")) {
            if (i == argc - 1) {
                printUsage();
                return 1;
            }
            cd.m_unTrPrefix = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(arg, "-project")) {
            if (i == argc - 1) {
                printErr("The option -project requires a parameter.\n"_L1);
                return 1;
            }
            if (!projectDescriptionFile.isEmpty()) {
                printErr("The option -project must appear only once.\n"_L1);
                return 1;
            }
            projectDescriptionFile = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(arg, "-silent")) {
            cd.m_verbose = false;
            continue;
        } else if (!strcmp(arg, "-verbose")) {
            cd.m_verbose = true;
            continue;
        } else if (!strcmp(arg, "-version")) {
            printOut("lrelease version %1\n"_L1.arg(QLatin1StringView(QT_VERSION_STR)));
            return 0;
        } else if (!strcmp(arg, "-qm")) {
            if (i == argc - 1) {
                printUsage();
                return 1;
            }
            outputFile = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(arg, "-help")) {
            printUsage();
            return 0;
        } else if (arg[0] == '-') {
            printUsage();
            return 1;
        } else {
            inputFiles << QString::fromLocal8Bit(arg);
        }
    }

    if (inputFiles.isEmpty() && projectDescriptionFile.isEmpty()) {
        printUsage();
        return 1;
    }

    QString errorString;
    if (!extractProFiles(&inputFiles).isEmpty()) {
        runInternalQtTool("lrelease-pro"_L1, app.arguments().mid(1));
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
            printErr("lrelease error: %1\n"_L1.arg(errorString));
            return 1;
        }
        inputFiles = translationsFromProjects(projectDescription);
    }

    for (const QString &inputFile : std::as_const(inputFiles)) {
        if (outputFile.isEmpty()) {
            if (!releaseTsFile(inputFile, cd, removeIdentical, failOnUnfinished))
                return 1;
        } else {
            if (!loadTsFile(tor, inputFile))
                return 1;
        }
    }

    if (!outputFile.isEmpty())
        return releaseTranslator(tor, outputFile, cd, removeIdentical, failOnUnfinished) ? 0 : 1;

    return 0;
}
