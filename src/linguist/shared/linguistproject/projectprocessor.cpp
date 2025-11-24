// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "projectprocessor.h"
#include "qrcreader.h"
#include <translator.h>
#include <trlib/trparser.h>

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qset.h>

#include <iostream>

using namespace Qt::StringLiterals;

namespace {

static void printOut(const QString &out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString &out)
{
    std::cerr << qPrintable(out);
}

static void printWarning(UpdateOptions options,
                const QString &msg,
                const QString &warningMsg = {},
                const QString &errorMsg = {})
{
    QString text = msg;
    if (options & Werror) {
        text.prepend("lupdate error: "_L1);
        if (!errorMsg.isEmpty())
            text.append(" "_L1).append(errorMsg);
    } else {
        text.prepend("lupdate warning: "_L1);
        if (!warningMsg.isEmpty())
            text.append(" "_L1).append(warningMsg);
    }

    printErr(text);
}

static bool readFileContent(const QString &filePath, QByteArray *content, QString *errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        *errorString = file.errorString();
        return false;
    }
    *content = file.readAll();
    return true;
}

static bool readFileContent(const QString &filePath, QString *content, QString *errorString)
{
    QByteArray ba;
    if (!readFileContent(filePath, &ba, errorString))
        return false;
    *content = QString::fromLocal8Bit(ba);
    return true;
}

static void removeExcludedSources(Projects &projects)
{
    for (Project &project : projects) {
        for (const QRegularExpression &rx : std::as_const(project.excluded)) {
            for (auto it = project.sources.begin(); it != project.sources.end(); ) {
                if (rx.match(*it).hasMatch())
                    it = project.sources.erase(it);
                else
                    ++it;
            }
        }
        removeExcludedSources(project.subProjects);
    }
}


// Remove .qrc files from the project and return them as absolute paths.
static QStringList extractQrcFiles(Project &project)
{
    auto it = project.sources.begin();
    QStringList qrcFiles;
    while (it != project.sources.end()) {
        QFileInfo fi(*it);
        QString fn = QDir::cleanPath(fi.absoluteFilePath());
        if (fn.endsWith(".qrc"_L1, Qt::CaseInsensitive)) {
            qrcFiles += fn;
            it = project.sources.erase(it);
        } else {
            ++it;
        }
    }
    return qrcFiles;
}

// Replace all .qrc files in the project with their content.
static void expandQrcFiles(Project &project)
{
    for (const QString &qrcFile : extractQrcFiles(project))
        project.sources << getSourceFilesFromQrc(qrcFile);
}

static QSet<QString> projectRoots(const QString &projectFile, const QStringList &sourceFiles)
{
    const QString proPath = QFileInfo(projectFile).path();
    QSet<QString> sourceDirs;
    sourceDirs.insert(proPath + u'/');
    for (const QString &sf : sourceFiles)
        sourceDirs.insert(sf.left(sf.lastIndexOf(u'/') + 1));
    QStringList rootList = sourceDirs.values();
    rootList.sort();
    for (int prev = 0, curr = 1; curr < rootList.size(); )
        if (rootList.at(curr).startsWith(rootList.at(prev)))
            rootList.removeAt(curr);
        else
            prev = curr++;
    return QSet<QString>(rootList.cbegin(), rootList.cend());
}

static bool updateTsFiles(const Translator &fetchedTor, const QStringList &tsFileNames,
    const QStringList &alienFiles,
    const QString &sourceLanguage, const QString &targetLanguage,
    UpdateOptions options)
{
    bool fail = false;
    for (int i = 0; i < fetchedTor.messageCount(); i++) {
        const TranslatorMessage &msg = fetchedTor.constMessage(i);
        if (!msg.id().isEmpty() && msg.sourceText().isEmpty()) {
            printWarning(options,
                         "Message with id '%1' has no source.\n"_L1.arg(msg.id()));
            if (options & Werror)
                return !fail;
        }
    }
    QList<Translator> aliens;
    for (const QString &fileName : alienFiles) {
        ConversionData cd;
        Translator tor;
        if (!tor.load(fileName, cd, "auto"_L1)) {
            printErr(cd.error());
            fail = true;
            continue;
        }
        tor.resolveDuplicates();
        aliens << tor;
    }
    QDir dir;
    QString err;
    for (const QString &fileName : tsFileNames) {
        QString fn = dir.relativeFilePath(fileName);
        ConversionData cd;
        Translator tor;
        cd.m_sortContexts = !(options & NoSort);
        cd.m_sortMessages = options & SortMessages;
        if (QFile(fileName).exists()) {
            if (!tor.load(fileName, cd, "auto"_L1)) {
                printErr(cd.error());
                fail = true;
                continue;
            }
            tor.resolveDuplicates();
            cd.clearErrors();
            if (!targetLanguage.isEmpty() && targetLanguage != tor.languageCode()) {
                printWarning(options,
                             "Specified target language '%1' disagrees with"
                                            " existing file's language '%2'.\n"_L1
                                     .arg(targetLanguage, tor.languageCode()),
                             u"Ignoring.\n"_s);
                if (options & Werror)
                    return !fail;
            }
            if (!sourceLanguage.isEmpty() && sourceLanguage != tor.sourceLanguageCode()) {
                printWarning(options,
                             "Specified source language '%1' disagrees with"
                                     " existing file's language '%2'.\n"_L1
                                     .arg(sourceLanguage, tor.sourceLanguageCode()),
                             u"Ignoring.\n"_s);
                if (options & Werror)
                    return !fail;
            }
            // If there is translation in the file, the language should be recognized
            // (when the language is not recognized, plural translations are lost)
            if (tor.translationsExist()) {

                if (tor.languageCode().isEmpty()) {
                    printErr("File %1 won't be updated: it does not specify any "
                             "target languages. To set a target language, open "
                             "the file in Qt Linguist.\n"_L1.arg(fileName));
                    continue;
                }
                QLocale::Language l;
                QLocale::Territory c;
                tor.languageAndTerritory(tor.languageCode(), &l, &c);
                QStringList forms;
                if (!getNumerusInfo(l, c, 0, &forms, 0)) {
                    printErr(QStringLiteral("File %1 won't be updated: it contains translation but the"
                    " target language is not recognized\n").arg(fileName));
                    continue;
                }
            }
        } else {
            if (!targetLanguage.isEmpty())
                tor.setLanguageCode(targetLanguage);
            else
                tor.setLanguageCode(Translator::guessLanguageCodeFromFileName(fileName));
            if (!sourceLanguage.isEmpty())
                tor.setSourceLanguageCode(sourceLanguage);
        }
        tor.makeFileNamesAbsolute(QFileInfo(fileName).absoluteDir());
        if (options & NoLocations)
            tor.setLocationsType(Translator::NoLocations);
        else if (options & RelativeLocations)
            tor.setLocationsType(Translator::RelativeLocations);
        else if (options & AbsoluteLocations)
            tor.setLocationsType(Translator::AbsoluteLocations);
        if (options & Verbose)
            printOut(QStringLiteral("Updating '%1'...\n").arg(fn));

        UpdateOptions theseOptions = options;
        if (tor.locationsType() == Translator::NoLocations) // Could be set from file
            theseOptions |= NoLocations;
        Translator out = merge(tor, fetchedTor, aliens, theseOptions, err);

        if ((options & Verbose) && !err.isEmpty()) {
            printOut(err);
            err.clear();
        }
        if (options & PluralOnly) {
            if (options & Verbose)
                printOut(QStringLiteral("Stripping non plural forms in '%1'...\n").arg(fn));
            out.stripNonPluralForms();
        }
        if (options & NoObsolete)
            out.stripObsoleteMessages();
        out.stripEmptyContexts();

        out.normalizeTranslations(cd);
        if (!cd.errors().isEmpty()) {
            printErr(cd.error());
            cd.clearErrors();
        }
        if (!out.save(fileName, cd, "auto"_L1)) {
            printErr(cd.error());
            fail = true;
        }
    }
    return !fail;
}

class ProjectProcessor
{
public:
    ProjectProcessor(const QString &sourceLanguage,
                     const QString &targetLanguage)
        : m_sourceLanguage(sourceLanguage),
          m_targetLanguage(targetLanguage)
    {
    }

    bool processProjects(bool topLevel, UpdateOptions options, const Projects &projects,
                         bool nestComplain, Translator *parentTor) const
    {
        bool ok = true;
        for (const Project &prj : projects)
            ok &= processProject(options, prj, topLevel, nestComplain, parentTor);
        return ok;
    }

private:

    bool processProject(UpdateOptions options, const Project &prj, bool topLevel,
                        bool nestComplain, Translator *parentTor) const
    {
        bool ok = true;
        QString codecForSource = prj.codec.toLower();
        if (!codecForSource.isEmpty()) {
            if (codecForSource == "utf-16"_L1 || codecForSource == "utf16"_L1) {
                options |= SourceIsUtf16;
            } else if (codecForSource == "utf-8"_L1 || codecForSource == "utf8"_L1) {
                options &= ~SourceIsUtf16;
            } else {
                printWarning(
                        options,
                        "Codec for source '%1' is invalid.\n"_L1.arg(codecForSource),
                        u"Falling back to UTF-8.\n"_s);
                if (options & Werror)
                    return ok;
                options &= ~SourceIsUtf16;
            }
        }

        const QString projectFile = prj.filePath;
        const QStringList sources = prj.sources;
        ConversionData cd;
        cd.m_noUiLines = options & NoUiLines;
        cd.m_projectRoots = projectRoots(projectFile, sources);
        QStringList projectRootDirs;
        for (const auto& dir : std::as_const(cd.m_projectRoots))
            projectRootDirs.append(dir);
        cd.m_includePath = prj.includePaths;
        cd.m_excludes = prj.excluded;
        cd.m_sourceIsUtf16 = options & SourceIsUtf16;

        QStringList tsFiles;
        if (prj.translations) {
            tsFiles = *prj.translations;
            if (parentTor) {
                if (topLevel) {
                    printWarning(options, u"Existing top level."_s,
                                 "TS files from command line will "
                                 "override TRANSLATIONS in %1.\n"_L1.arg(projectFile),
                                 u"Terminating the operation.\n"_s);
                    if (options & Werror)
                        return ok;
                    goto noTrans;
                } else if (nestComplain) {
                    printWarning(options,
                                "TS files from command line "
                                "prevent recursing into %1.\n"_L1.arg(projectFile));
                    return ok;
                }
            }
            if (tsFiles.isEmpty()) {
                // This might mean either a buggy PRO file or an intentional detach -
                // we can't know without seeing the actual RHS of the assignment ...
                // Just assume correctness and be silent.
                return ok;
            }
            Translator tor;
            ok = processProjects(false, options, prj.subProjects, false, &tor);
            processSources(tor, sources, cd);
            ok &= updateTsFiles(tor, tsFiles, QStringList(), m_sourceLanguage, m_targetLanguage,
                          options);
            return ok;
        }


      noTrans:
        if (!parentTor) {
            if (topLevel) {
                printWarning(options, u"no TS files specified."_s,
                             "Only diagnostics will be produced for %1.\n"_L1.arg(projectFile),
                             u"Terminating the operation.\n"_s);
                if (options & Werror)
                    return ok;
            }
            Translator tor;
            ok = processProjects(false, options, prj.subProjects, nestComplain, &tor);
            processSources(tor, sources, cd);
        } else {
            ok = processProjects(false, options, prj.subProjects, nestComplain, parentTor);
            processSources(*parentTor, sources, cd);
        }
        return ok;
    }

    QString m_sourceLanguage;
    QString m_targetLanguage;
};

}

QT_BEGIN_NAMESPACE

QStringList getSourceFilesFromQrc(const QString &resourceFile)
{
    if (!QFile::exists(resourceFile))
        return QStringList();
    QString content;
    QString errStr;
    if (!readFileContent(resourceFile, &content, &errStr)) {
        printErr(QStringLiteral("lupdate error: Can not read %1: %2\n").arg(resourceFile, errStr));
        return QStringList();
    }
    ReadQrcResult rqr = readQrcFile(resourceFile, content);
    if (rqr.hasError()) {
        printErr(QStringLiteral("lupdate error: %1:%2: %3\n")
                         .arg(resourceFile, QString::number(rqr.line), rqr.errorString));
    }
    return rqr.files;
}

bool processProjectDescription(
    Projects &projectDescription,
    const QStringList &tsFileNames,
    const QStringList &alienFiles,
    const QString &sourceLanguage,
    const QString &targetLanguage,
    UpdateOptions options)
{
    bool ok = true;

    removeExcludedSources(projectDescription);
    for (Project &project : projectDescription)
        expandQrcFiles(project);

    ProjectProcessor projectProcessor(sourceLanguage, targetLanguage);
    if (!tsFileNames.isEmpty()) {
        Translator fetchedTor;
        ok &= projectProcessor.processProjects(true, options, projectDescription, true, &fetchedTor);
        if (ok) {
            ok &= updateTsFiles(fetchedTor, tsFileNames, alienFiles,
                          sourceLanguage, targetLanguage, options);
        }
    } else {
        ok &= projectProcessor.processProjects(true, options, projectDescription, false, nullptr);
    }
    return ok;
}

bool processSourceFiles(
    const QStringList &sourceFiles,
    const QStringList &tsFileNames,
    const QStringList &alienFiles,
    const QSet<QString> &projectRoots,
    const QStringList &includePath,
    const QMultiHash<QString, QString> &allCSources,
    const QString &sourceLanguage,
    const QString &targetLanguage,
    UpdateOptions options)
{
    Translator fetchedTor;
    ConversionData cd;
    cd.m_noUiLines = options & NoUiLines;
    cd.m_sourceIsUtf16 = options & SourceIsUtf16;
    cd.m_projectRoots = projectRoots;
    cd.m_includePath = includePath;
    cd.m_allCSources = allCSources;
    processSources(fetchedTor, sourceFiles, cd);
    return updateTsFiles(fetchedTor, tsFileNames, alienFiles,
                  sourceLanguage, targetLanguage, options);
}

QT_END_NAMESPACE
