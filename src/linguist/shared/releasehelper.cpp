// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "releasehelper.h"
#include "translator.h"
#include "validator.h"
#include "linguistproject/projectdescriptionreader.h"

#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>

#include <iostream>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

void printOut(const QString &out)
{
    std::cout << qPrintable(out);
}

void printErr(const QString &out)
{
    std::cerr << qPrintable(out);
}

bool loadTsFile(Translator &tor, const QString &tsFileName)
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

bool releaseTranslator(Translator &tor, const QString &qmFileName, ConversionData &cd,
                       ParamFlags params)
{
    if (params.failOnUnfinished && tor.unfinishedTranslationsExist()) {
        printErr("lrelease error: cannot create '%1': existing unfinished translation(s) "
                 "found (-fail-on-unfinished)"_L1.arg(qmFileName));
        return false;
    }

    if (params.failOnInvalid) {
        Validator::Checks checks{ true, true, true, true };
        QLocale::Language sourceLang;
        QLocale::Language targetLang;
        QLocale::Territory targetTerritory;
        QList<bool> countRefNeeds;
        tor.languageAndTerritory(tor.sourceLanguageCode(), &sourceLang, nullptr);
        tor.languageAndTerritory(tor.languageCode(), &targetLang, &targetTerritory);
        QMap<Validator::ErrorType, QString> errors;
        if (getCountNeed(targetLang, targetTerritory, countRefNeeds, nullptr))
            for (const TranslatorMessage &msg : tor.messages()) {
                if (msg.isTranslated() && msg.type() != TranslatorMessage::Finished) {
                    Validator validator =
                            Validator::fromSource(msg.sourceText(), checks, sourceLang, {});
                    errors.insert(
                            validator.validate(msg.translations(), msg, targetLang, countRefNeeds));
                }
            }
        else
            printErr("Could not get numerus info"_L1);
        if (cd.isVerbose())
            for (const QString &trs : errors)
                printErr("Validation error for translation '%1'"_L1.arg(trs));
        if (!errors.empty())
            return false;
    }

    tor.reportDuplicates(tor.resolveDuplicates(), qmFileName, cd.isVerbose());

    if (cd.isVerbose())
        printOut("Updating '%1'...\n"_L1.arg(qmFileName));
    if (params.removeIdentical) {
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

bool releaseTsFile(const QString &tsFileName, ConversionData &cd, ParamFlags params)
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

    return releaseTranslator(tor, qmFileName, cd, params);
}

QStringList translationsFromProject(const Project &project, bool topLevel)
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

QStringList translationsFromProjects(const Projects &projects, bool topLevel)
{
    QStringList result;
    for (const Project &p : projects)
        result << translationsFromProject(p, topLevel);
    return result;
}

QT_END_NAMESPACE
