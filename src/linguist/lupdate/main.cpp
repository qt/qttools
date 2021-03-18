/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Marc Mutz <marc.mutz@kdab.com>
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

#include "lupdate.h"

#include <profileutils.h>
#include <projectdescriptionreader.h>
#include <qrcreader.h>
#include <runqttool.h>
#include <translator.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>

#include <iostream>

// Can't have an array of QStaticStringData<N> for different N, so
// use QString, which requires constructor calls. Doesn't matter
// much, since this is in an app, not a lib:
static const QString defaultTrFunctionNames[] = {
// MSVC can't handle the lambda in this array if QStringLiteral expands
// to a lambda. In that case, use a QString instead.
#if defined(Q_CC_MSVC) && defined(Q_COMPILER_LAMBDA)
#define STRINGLITERAL(F) QLatin1String(#F),
#else
#define STRINGLITERAL(F) QStringLiteral(#F),
#endif
    LUPDATE_FOR_EACH_TR_FUNCTION(STRINGLITERAL)
#undef STRINGLITERAL
};
Q_STATIC_ASSERT((TrFunctionAliasManager::NumTrFunctions == sizeof defaultTrFunctionNames / sizeof *defaultTrFunctionNames));

static int trFunctionByDefaultName(const QString &trFunctionName)
{
    for (int i = 0; i < TrFunctionAliasManager::NumTrFunctions; ++i)
        if (trFunctionName == defaultTrFunctionNames[i])
            return i;
    return -1;
}

TrFunctionAliasManager::TrFunctionAliasManager()
    : m_trFunctionAliases()
{
    for (int i = 0; i < NumTrFunctions; ++i)
        m_trFunctionAliases[i].push_back(defaultTrFunctionNames[i]);
}

TrFunctionAliasManager::~TrFunctionAliasManager() {}

int TrFunctionAliasManager::trFunctionByName(const QString &trFunctionName) const
{
    ensureTrFunctionHashUpdated();
    // this function needs to be fast
    const QHash<QString, TrFunction>::const_iterator it
        = m_nameToTrFunctionMap.find(trFunctionName);
    return it == m_nameToTrFunctionMap.end() ? -1 : *it;
}

void TrFunctionAliasManager::modifyAlias(int trFunction, const QString &alias, Operation op)
{
    QList<QString> &list = m_trFunctionAliases[trFunction];
    if (op == SetAlias)
        list.clear();
    list.push_back(alias);
    m_nameToTrFunctionMap.clear();
}

void TrFunctionAliasManager::ensureTrFunctionHashUpdated() const
{
    if (!m_nameToTrFunctionMap.empty())
        return;

    QHash<QString, TrFunction> nameToTrFunctionMap;
    for (int i = 0; i < NumTrFunctions; ++i)
        foreach (const QString &alias, m_trFunctionAliases[i])
            nameToTrFunctionMap[alias] = TrFunction(i);
    // commit:
    m_nameToTrFunctionMap.swap(nameToTrFunctionMap);
}

static QStringList availableFunctions()
{
    QStringList result;
    result.reserve(TrFunctionAliasManager::NumTrFunctions);
    for (int i = 0; i < TrFunctionAliasManager::NumTrFunctions; ++i)
        result.push_back(defaultTrFunctionNames[i]);
    return result;
}

QStringList TrFunctionAliasManager::availableFunctionsWithAliases() const
{
    QStringList result;
    result.reserve(NumTrFunctions);
    for (int i = 0; i < NumTrFunctions; ++i)
        result.push_back(defaultTrFunctionNames[i] +
                         QLatin1String(" (=") +
                         m_trFunctionAliases[i].join(QLatin1Char('=')) +
                         QLatin1Char(')'));
    return result;
}

TrFunctionAliasManager trFunctionAliasManager;

QString ParserTool::transcode(const QString &str)
{
    static const char tab[] = "abfnrtv";
    static const char backTab[] = "\a\b\f\n\r\t\v";
    // This function has to convert back to bytes, as C's \0* sequences work at that level.
    const QByteArray in = str.toUtf8();
    QByteArray out;

    out.reserve(in.length());
    for (int i = 0; i < in.length();) {
        uchar c = in[i++];
        if (c == '\\') {
            if (i >= in.length())
                break;
            c = in[i++];

            if (c == '\n')
                continue;

            if (c == 'x' || c == 'u' || c == 'U') {
                const bool unicode = (c != 'x');
                QByteArray hex;
                while (i < in.length() && isxdigit((c = in[i]))) {
                    hex += c;
                    i++;
                }
                if (unicode)
                    out += QString(QChar(hex.toUInt(nullptr, 16))).toUtf8();
                else
                    out += hex.toUInt(nullptr, 16);
            } else if (c >= '0' && c < '8') {
                QByteArray oct;
                int n = 0;
                oct += c;
                while (n < 2 && i < in.length() && (c = in[i]) >= '0' && c < '8') {
                    i++;
                    n++;
                    oct += c;
                }
                out += oct.toUInt(0, 8);
            } else {
                const char *p = strchr(tab, c);
                out += !p ? c : backTab[p - tab];
            }
        } else {
            out += c;
        }
    }
    return QString::fromUtf8(out.constData(), out.length());
}

static QString m_defaultExtensions;

static void printOut(const QString & out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString & out)
{
    std::cerr << qPrintable(out);
}

static void recursiveFileInfoList(const QDir &dir,
    const QSet<QString> &nameFilters, QDir::Filters filter,
    QFileInfoList *fileinfolist)
{
    foreach (const QFileInfo &fi, dir.entryInfoList(filter))
        if (fi.isDir())
            recursiveFileInfoList(QDir(fi.absoluteFilePath()), nameFilters, filter, fileinfolist);
        else if (nameFilters.contains(fi.suffix()))
            fileinfolist->append(fi);
}

static void printUsage()
{
    printOut(LU::tr(
        "Usage:\n"
        "    lupdate [options] [project-file]...\n"
        "    lupdate [options] [source-file|path|@lst-file]... -ts ts-files|@lst-file\n\n"
        "lupdate is part of Qt's Linguist tool chain. It extracts translatable\n"
        "messages from Qt UI files, C++, Java and JavaScript/QtScript source code.\n"
        "Extracted messages are stored in textual translation source files (typically\n"
        "Qt TS XML). New and modified messages can be merged into existing TS files.\n\n"
        "Passing .pro files to lupdate is deprecated.\n"
        "Please use the lupdate-pro tool instead.\n\n"
        "Options:\n"
        "    -help  Display this information and exit.\n"
        "    -no-obsolete\n"
        "           Drop all obsolete and vanished strings.\n"
        "    -extensions <ext>[,<ext>]...\n"
        "           Process files with the given extensions only.\n"
        "           The extension list must be separated with commas, not with whitespace.\n"
        "           Default: '%1'.\n"
        "    -pluralonly\n"
        "           Only include plural form messages.\n"
        "    -silent\n"
        "           Do not explain what is being done.\n"
        "    -no-sort\n"
        "           Do not sort contexts in TS files.\n"
        "    -no-recursive\n"
        "           Do not recursively scan directories.\n"
        "    -recursive\n"
        "           Recursively scan directories (default).\n"
        "    -I <includepath> or -I<includepath>\n"
        "           Additional location to look for include files.\n"
        "           May be specified multiple times.\n"
        "    -locations {absolute|relative|none}\n"
        "           Specify/override how source code references are saved in TS files.\n"
        "           Guessed from existing TS files if not specified.\n"
        "           Default is absolute for new files.\n"
        "    -no-ui-lines\n"
        "           Do not record line numbers in references to UI files.\n"
        "    -disable-heuristic {sametext|similartext|number}\n"
        "           Disable the named merge heuristic. Can be specified multiple times.\n"
        "    -project <filename>\n"
        "           Name of a file containing the project's description in JSON format.\n"
        "           Such a file may be generated from a .pro file using the lprodump tool.\n"
        "    -pro <filename>\n"
        "           Name of a .pro file. Useful for files with .pro file syntax but\n"
        "           different file suffix. Projects are recursed into and merged.\n"
        "           This option is deprecated. Use the lupdate-pro tool instead.\n"
        "    -pro-out <directory>\n"
        "           Virtual output directory for processing subsequent .pro files.\n"
        "    -pro-debug\n"
        "           Trace processing .pro files. Specify twice for more verbosity.\n"
        "    -source-language <language>[_<region>]\n"
        "           Specify the language of the source strings for new files.\n"
        "           Defaults to POSIX if not specified.\n"
        "    -target-language <language>[_<region>]\n"
        "           Specify the language of the translations for new files.\n"
        "           Guessed from the file name if not specified.\n"
        "    -tr-function-alias <function>{+=,=}<alias>[,<function>{+=,=}<alias>]...\n"
        "           With +=, recognize <alias> as an alternative spelling of <function>.\n"
        "           With  =, recognize <alias> as the only spelling of <function>.\n"
        "           Available <function>s (with their currently defined aliases) are:\n"
        "             %2\n"
        "    -ts <ts-file>...\n"
        "           Specify the output file(s). This will override the TRANSLATIONS.\n"
        "    -version\n"
        "           Display the version of lupdate and exit.\n"
        "    @lst-file\n"
        "           Read additional file names (one per line) or includepaths (one per\n"
        "           line, and prefixed with -I) from lst-file.\n"
    ).arg(m_defaultExtensions,
          trFunctionAliasManager.availableFunctionsWithAliases()
                                .join(QLatin1String("\n             "))));
}

static bool handleTrFunctionAliases(const QString &arg)
{
    foreach (const QString &pair, arg.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const int equalSign = pair.indexOf(QLatin1Char('='));
        if (equalSign < 0) {
            printErr(LU::tr("tr-function mapping '%1' in -tr-function-alias is missing the '='.\n").arg(pair));
            return false;
        }
        const bool plusEqual = equalSign > 0 && pair[equalSign-1] == QLatin1Char('+');
        const int trFunctionEnd = plusEqual ? equalSign-1 : equalSign;
        const QString trFunctionName = pair.left(trFunctionEnd).trimmed();
        const QString alias = pair.mid(equalSign+1).trimmed();
        const int trFunction = trFunctionByDefaultName(trFunctionName);
        if (trFunction < 0) {
            printErr(LU::tr("Unknown tr-function '%1' in -tr-function-alias option.\n"
                            "Available tr-functions are: %2")
                     .arg(trFunctionName, availableFunctions().join(QLatin1Char(','))));
            return false;
        }
        if (alias.isEmpty()) {
            printErr(LU::tr("Empty alias for tr-function '%1' in -tr-function-alias option.\n")
                     .arg(trFunctionName));
            return false;
        }
        trFunctionAliasManager.modifyAlias(trFunction, alias,
                                           plusEqual ? TrFunctionAliasManager::AddAlias : TrFunctionAliasManager::SetAlias);
    }
    return true;
}

static void updateTsFiles(const Translator &fetchedTor, const QStringList &tsFileNames,
    const QStringList &alienFiles,
    const QString &sourceLanguage, const QString &targetLanguage,
    UpdateOptions options, bool *fail)
{
    for (int i = 0; i < fetchedTor.messageCount(); i++) {
        const TranslatorMessage &msg = fetchedTor.constMessage(i);
        if (!msg.id().isEmpty() && msg.sourceText().isEmpty())
            printErr(LU::tr("lupdate warning: Message with id '%1' has no source.\n")
                     .arg(msg.id()));
    }

    QList<Translator> aliens;
    foreach (const QString &fileName, alienFiles) {
        ConversionData cd;
        Translator tor;
        if (!tor.load(fileName, cd, QLatin1String("auto"))) {
            printErr(cd.error());
            *fail = true;
            continue;
        }
        tor.resolveDuplicates();
        aliens << tor;
    }

    QDir dir;
    QString err;
    foreach (const QString &fileName, tsFileNames) {
        QString fn = dir.relativeFilePath(fileName);
        ConversionData cd;
        Translator tor;
        cd.m_sortContexts = !(options & NoSort);
        if (QFile(fileName).exists()) {
            if (!tor.load(fileName, cd, QLatin1String("auto"))) {
                printErr(cd.error());
                *fail = true;
                continue;
            }
            tor.resolveDuplicates();
            cd.clearErrors();
            if (!targetLanguage.isEmpty() && targetLanguage != tor.languageCode())
                printErr(LU::tr("lupdate warning: Specified target language '%1' disagrees with"
                                " existing file's language '%2'. Ignoring.\n")
                         .arg(targetLanguage, tor.languageCode()));
            if (!sourceLanguage.isEmpty() && sourceLanguage != tor.sourceLanguageCode())
                printErr(LU::tr("lupdate warning: Specified source language '%1' disagrees with"
                                " existing file's language '%2'. Ignoring.\n")
                         .arg(sourceLanguage, tor.sourceLanguageCode()));
            // If there is translation in the file, the language should be recognized
            // (when the language is not recognized, plural translations are lost)
            if (tor.translationsExist()) {
                QLocale::Language l;
                QLocale::Country c;
                tor.languageAndCountry(tor.languageCode(), &l, &c);
                QStringList forms;
                if (!getNumerusInfo(l, c, 0, &forms, 0)) {
                    printErr(LU::tr("File %1 won't be updated: it contains translation but the"
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
            printOut(LU::tr("Updating '%1'...\n").arg(fn));

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
                printOut(LU::tr("Stripping non plural forms in '%1'...\n").arg(fn));
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
        if (!out.save(fileName, cd, QLatin1String("auto"))) {
            printErr(cd.error());
            *fail = true;
        }
    }
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

static QStringList getResources(const QString &resourceFile)
{
    if (!QFile::exists(resourceFile))
        return QStringList();
    QString content;
    QString errStr;
    if (!readFileContent(resourceFile, &content, &errStr)) {
        printErr(LU::tr("lupdate error: Can not read %1: %2\n").arg(resourceFile, errStr));
        return QStringList();
    }
    ReadQrcResult rqr = readQrcFile(resourceFile, content);
    if (rqr.hasError()) {
        printErr(LU::tr("lupdate error: %1:%2: %3\n")
                 .arg(resourceFile, QString::number(rqr.line), rqr.errorString));
    }
    return rqr.files;
}

static bool processTs(Translator &fetchedTor, const QString &file, ConversionData &cd)
{
    foreach (const Translator::FileFormat &fmt, Translator::registeredFileFormats()) {
        if (file.endsWith(QLatin1Char('.') + fmt.extension, Qt::CaseInsensitive)) {
            Translator tor;
            if (tor.load(file, cd, fmt.extension)) {
                foreach (TranslatorMessage msg, tor.messages()) {
                    msg.setType(TranslatorMessage::Unfinished);
                    msg.setTranslations(QStringList());
                    msg.setTranslatorComment(QString());
                    fetchedTor.extend(msg, cd);
                }
            }
            return true;
        }
    }
    return false;
}

static void processSources(Translator &fetchedTor,
                           const QStringList &sourceFiles, ConversionData &cd)
{
#ifdef QT_NO_QML
    bool requireQmlSupport = false;
#endif
    QStringList sourceFilesCpp;
    for (QStringList::const_iterator it = sourceFiles.begin(); it != sourceFiles.end(); ++it) {
        if (it->endsWith(QLatin1String(".java"), Qt::CaseInsensitive))
            loadJava(fetchedTor, *it, cd);
        else if (it->endsWith(QLatin1String(".ui"), Qt::CaseInsensitive)
                 || it->endsWith(QLatin1String(".jui"), Qt::CaseInsensitive))
            loadUI(fetchedTor, *it, cd);
#ifndef QT_NO_QML
        else if (it->endsWith(QLatin1String(".js"), Qt::CaseInsensitive)
                 || it->endsWith(QLatin1String(".qs"), Qt::CaseInsensitive))
            loadQScript(fetchedTor, *it, cd);
        else if (it->endsWith(QLatin1String(".qml"), Qt::CaseInsensitive))
            loadQml(fetchedTor, *it, cd);
#else
        else if (it->endsWith(QLatin1String(".qml"), Qt::CaseInsensitive)
                 || it->endsWith(QLatin1String(".js"), Qt::CaseInsensitive)
                 || it->endsWith(QLatin1String(".qs"), Qt::CaseInsensitive))
            requireQmlSupport = true;
#endif // QT_NO_QML
        else if (!processTs(fetchedTor, *it, cd))
            sourceFilesCpp << *it;
    }

#ifdef QT_NO_QML
    if (requireQmlSupport)
        printErr(LU::tr("lupdate warning: Some files have been ignored due to missing qml/javascript support\n"));
#endif

    loadCPP(fetchedTor, sourceFilesCpp, cd);
    if (!cd.error().isEmpty())
        printErr(cd.error());
}

static QSet<QString> projectRoots(const QString &projectFile, const QStringList &sourceFiles)
{
    const QString proPath = QFileInfo(projectFile).path();
    QSet<QString> sourceDirs;
    sourceDirs.insert(proPath + QLatin1Char('/'));
    for (const QString &sf : sourceFiles)
        sourceDirs.insert(sf.left(sf.lastIndexOf(QLatin1Char('/')) + 1));
    QStringList rootList = sourceDirs.values();
    rootList.sort();
    for (int prev = 0, curr = 1; curr < rootList.length(); )
        if (rootList.at(curr).startsWith(rootList.at(prev)))
            rootList.removeAt(curr);
        else
            prev = curr++;
    return QSet<QString>(rootList.cbegin(), rootList.cend());
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

    void processProjects(bool topLevel, UpdateOptions options, const Projects &projects,
                         bool nestComplain, Translator *parentTor, bool *fail) const
    {
        for (const Project &prj : projects)
            processProject(options, prj, topLevel, nestComplain, parentTor, fail);
    }

private:
    void processProject(UpdateOptions options, const Project &prj, bool topLevel,
                        bool nestComplain, Translator *parentTor, bool *fail) const
    {
        QString codecForSource = prj.codec.toLower();
        if (!codecForSource.isEmpty()) {
            if (codecForSource == QLatin1String("utf-16")
                || codecForSource == QLatin1String("utf16")) {
                options |= SourceIsUtf16;
            } else if (codecForSource == QLatin1String("utf-8")
                       || codecForSource == QLatin1String("utf8")) {
                options &= ~SourceIsUtf16;
            } else {
                printErr(LU::tr("lupdate warning: Codec for source '%1' is invalid."
                                " Falling back to UTF-8.\n").arg(codecForSource));
                options &= ~SourceIsUtf16;
            }
        }

        const QString projectFile = prj.filePath;
        const QStringList sources = prj.sources;
        ConversionData cd;
        cd.m_noUiLines = options & NoUiLines;
        cd.m_projectRoots = projectRoots(projectFile, sources);
        cd.m_includePath = prj.includePaths;
        cd.m_excludes = prj.excluded;
        cd.m_sourceIsUtf16 = options & SourceIsUtf16;

        QStringList tsFiles;
        if (hasTranslations(prj)) {
            tsFiles = *prj.translations;
            if (parentTor) {
                if (topLevel) {
                    printErr(LU::tr("lupdate warning: TS files from command line "
                                    "will override TRANSLATIONS in %1.\n").arg(projectFile));
                    goto noTrans;
                } else if (nestComplain) {
                    printErr(LU::tr("lupdate warning: TS files from command line "
                                    "prevent recursing into %1.\n").arg(projectFile));
                    return;
                }
            }
            if (tsFiles.isEmpty()) {
                // This might mean either a buggy PRO file or an intentional detach -
                // we can't know without seeing the actual RHS of the assignment ...
                // Just assume correctness and be silent.
                return;
            }
            Translator tor;
            processProjects(false, options, prj.subProjects, false, &tor, fail);
            processSources(tor, sources, cd);
            updateTsFiles(tor, tsFiles, QStringList(), m_sourceLanguage, m_targetLanguage,
                          options, fail);
            return;
        }

      noTrans:
        if (!parentTor) {
            if (topLevel) {
                printErr(LU::tr("lupdate warning: no TS files specified. Only diagnostics "
                                "will be produced for '%1'.\n").arg(projectFile));
            }
            Translator tor;
            processProjects(false, options, prj.subProjects, nestComplain, &tor, fail);
            processSources(tor, sources, cd);
        } else {
            processProjects(false, options, prj.subProjects, nestComplain, parentTor, fail);
            processSources(*parentTor, sources, cd);
        }
    }

    QString m_sourceLanguage;
    QString m_targetLanguage;
};

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

    m_defaultExtensions = QLatin1String("java,jui,ui,c,c++,cc,cpp,cxx,ch,h,h++,hh,hpp,hxx,js,qs,qml,qrc");

    QStringList args = app.arguments();
    QStringList tsFileNames;
    QStringList proFiles;
    QString projectDescriptionFile;
    QString outDir = QDir::currentPath();
    QMultiHash<QString, QString> allCSources;
    QSet<QString> projectRoots;
    QStringList sourceFiles;
    QStringList resourceFiles;
    QStringList includePath;
    QStringList alienFiles;
    QString targetLanguage;
    QString sourceLanguage;

    UpdateOptions options =
        Verbose | // verbose is on by default starting with Qt 4.2
        HeuristicSameText | HeuristicSimilarText | HeuristicNumber;
    int proDebug = 0;
    int numFiles = 0;
    bool metTsFlag = false;
    bool metXTsFlag = false;
    bool recursiveScan = true;

    QString extensions = m_defaultExtensions;
    QSet<QString> extensionsNameFilters;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == QLatin1String("-help")
                || arg == QLatin1String("--help")
                || arg == QLatin1String("-h")) {
            printUsage();
            return 0;
        } else if (arg == QLatin1String("-list-languages")) {
            printOut(getNumerusInfoString());
            return 0;
        } else if (arg == QLatin1String("-pluralonly")) {
            options |= PluralOnly;
            continue;
        } else if (arg == QLatin1String("-noobsolete")
                || arg == QLatin1String("-no-obsolete")) {
            options |= NoObsolete;
            continue;
        } else if (arg == QLatin1String("-silent")) {
            options &= ~Verbose;
            continue;
        } else if (arg == QLatin1String("-pro-debug")) {
            proDebug++;
            continue;
        } else if (arg == QLatin1String("-project")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The option -project requires a parameter.\n"));
                return 1;
            }
            if (!projectDescriptionFile.isEmpty()) {
                printErr(LU::tr("The option -project must appear only once.\n"));
                return 1;
            }
            projectDescriptionFile = args[i];
            numFiles++;
            continue;
        } else if (arg == QLatin1String("-target-language")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The option -target-language requires a parameter.\n"));
                return 1;
            }
            targetLanguage = args[i];
            continue;
        } else if (arg == QLatin1String("-source-language")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The option -source-language requires a parameter.\n"));
                return 1;
            }
            sourceLanguage = args[i];
            continue;
        } else if (arg == QLatin1String("-disable-heuristic")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The option -disable-heuristic requires a parameter.\n"));
                return 1;
            }
            arg = args[i];
            if (arg == QLatin1String("sametext")) {
                options &= ~HeuristicSameText;
            } else if (arg == QLatin1String("similartext")) {
                options &= ~HeuristicSimilarText;
            } else if (arg == QLatin1String("number")) {
                options &= ~HeuristicNumber;
            } else {
                printErr(LU::tr("Invalid heuristic name passed to -disable-heuristic.\n"));
                return 1;
            }
            continue;
        } else if (arg == QLatin1String("-locations")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The option -locations requires a parameter.\n"));
                return 1;
            }
            if (args[i] == QLatin1String("none")) {
                options |= NoLocations;
            } else if (args[i] == QLatin1String("relative")) {
                options |= RelativeLocations;
            } else if (args[i] == QLatin1String("absolute")) {
                options |= AbsoluteLocations;
            } else {
                printErr(LU::tr("Invalid parameter passed to -locations.\n"));
                return 1;
            }
            continue;
        } else if (arg == QLatin1String("-no-ui-lines")) {
            options |= NoUiLines;
            continue;
        } else if (arg == QLatin1String("-verbose")) {
            options |= Verbose;
            continue;
        } else if (arg == QLatin1String("-no-recursive")) {
            recursiveScan = false;
            continue;
        } else if (arg == QLatin1String("-recursive")) {
            recursiveScan = true;
            continue;
        } else if (arg == QLatin1String("-no-sort")
                   || arg == QLatin1String("-nosort")) {
            options |= NoSort;
            continue;
        } else if (arg == QLatin1String("-version")) {
            printOut(LU::tr("lupdate version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == QLatin1String("-ts")) {
            metTsFlag = true;
            metXTsFlag = false;
            continue;
        } else if (arg == QLatin1String("-xts")) {
            metTsFlag = false;
            metXTsFlag = true;
            continue;
        } else if (arg == QLatin1String("-extensions")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The -extensions option should be followed by an extension list.\n"));
                return 1;
            }
            extensions = args[i];
            continue;
        } else if (arg == QLatin1String("-tr-function-alias")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The -tr-function-alias option should be followed by a list of function=alias mappings.\n"));
                return 1;
            }
            if (!handleTrFunctionAliases(args[i]))
                return 1;
            continue;
        } else if (arg == QLatin1String("-pro")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The -pro option should be followed by a filename of .pro file.\n"));
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            numFiles++;
            continue;
        } else if (arg == QLatin1String("-pro-out")) {
            ++i;
            if (i == argc) {
                printErr(LU::tr("The -pro-out option should be followed by a directory name.\n"));
                return 1;
            }
            outDir = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            continue;
        } else if (arg.startsWith(QLatin1String("-I"))) {
            if (arg.length() == 2) {
                ++i;
                if (i == argc) {
                    printErr(LU::tr("The -I option should be followed by a path.\n"));
                    return 1;
                }
                includePath += args[i];
            } else {
                includePath += args[i].mid(2);
            }
            continue;
        } else if (arg.startsWith(QLatin1String("-")) && arg != QLatin1String("-")) {
            printErr(LU::tr("Unrecognized option '%1'.\n").arg(arg));
            return 1;
        }

        QStringList files;
        if (arg.startsWith(QLatin1String("@"))) {
            QFile lstFile(arg.mid(1));
            if (!lstFile.open(QIODevice::ReadOnly)) {
                printErr(LU::tr("lupdate error: List file '%1' is not readable.\n")
                         .arg(lstFile.fileName()));
                return 1;
            }
            while (!lstFile.atEnd()) {
                QString lineContent = QString::fromLocal8Bit(lstFile.readLine().trimmed());

                if (lineContent.startsWith(QLatin1String("-I"))) {
                    if (lineContent.length() == 2) {
                        printErr(LU::tr("The -I option should be followed by a path.\n"));
                        return 1;
                    }
                    includePath += lineContent.mid(2);
                } else {
                    files << lineContent;
                }
            }
        } else {
            files << arg;
        }
        if (metTsFlag) {
            foreach (const QString &file, files) {
                bool found = false;
                foreach (const Translator::FileFormat &fmt, Translator::registeredFileFormats()) {
                    if (file.endsWith(QLatin1Char('.') + fmt.extension, Qt::CaseInsensitive)) {
                        QFileInfo fi(file);
                        if (!fi.exists() || fi.isWritable()) {
                            tsFileNames.append(QFileInfo(file).absoluteFilePath());
                        } else {
                            printErr(LU::tr("lupdate warning: For some reason, '%1' is not writable.\n")
                                     .arg(file));
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    printErr(LU::tr("lupdate error: File '%1' has no recognized extension.\n")
                             .arg(file));
                    return 1;
                }
            }
            numFiles++;
        } else if (metXTsFlag) {
            alienFiles += files;
        } else {
            foreach (const QString &file, files) {
                QFileInfo fi(file);
                if (!fi.exists()) {
                    printErr(LU::tr("lupdate error: File '%1' does not exist.\n").arg(file));
                    return 1;
                }
                if (isProOrPriFile(file)) {
                    QString cleanFile = QDir::cleanPath(fi.absoluteFilePath());
                    proFiles << cleanFile;
                } else if (fi.isDir()) {
                    if (options & Verbose)
                        printOut(LU::tr("Scanning directory '%1'...\n").arg(file));
                    QDir dir = QDir(fi.filePath());
                    projectRoots.insert(dir.absolutePath() + QLatin1Char('/'));
                    if (extensionsNameFilters.isEmpty()) {
                        foreach (QString ext, extensions.split(QLatin1Char(','))) {
                            ext = ext.trimmed();
                            if (ext.startsWith(QLatin1Char('.')))
                                ext.remove(0, 1);
                            extensionsNameFilters.insert(ext);
                        }
                    }
                    QDir::Filters filters = QDir::Files | QDir::NoSymLinks;
                    if (recursiveScan)
                        filters |= QDir::AllDirs | QDir::NoDotAndDotDot;
                    QFileInfoList fileinfolist;
                    recursiveFileInfoList(dir, extensionsNameFilters, filters, &fileinfolist);
                    int scanRootLen = dir.absolutePath().length();
                    foreach (const QFileInfo &fi, fileinfolist) {
                        QString fn = QDir::cleanPath(fi.absoluteFilePath());
                        if (fn.endsWith(QLatin1String(".qrc"), Qt::CaseInsensitive)) {
                            resourceFiles << fn;
                        } else {
                            sourceFiles << fn;

                            if (!fn.endsWith(QLatin1String(".java"))
                                    && !fn.endsWith(QLatin1String(".jui"))
                                    && !fn.endsWith(QLatin1String(".ui"))
                                    && !fn.endsWith(QLatin1String(".js"))
                                    && !fn.endsWith(QLatin1String(".qs"))
                                    && !fn.endsWith(QLatin1String(".qml"))) {
                                int offset = 0;
                                int depth = 0;
                                do {
                                    offset = fn.lastIndexOf(QLatin1Char('/'), offset - 1);
                                    QString ffn = fn.mid(offset + 1);
                                    allCSources.insert(ffn, fn);
                                } while (++depth < 3 && offset > scanRootLen);
                            }
                        }
                    }
                } else {
                    QString fn = QDir::cleanPath(fi.absoluteFilePath());
                    if (fn.endsWith(QLatin1String(".qrc"), Qt::CaseInsensitive))
                        resourceFiles << fn;
                    else
                        sourceFiles << fn;
                    projectRoots.insert(fi.absolutePath() + QLatin1Char('/'));
                }
            }
            numFiles++;
        }
    } // for args

    if (numFiles == 0) {
        printUsage();
        return 1;
    }

    if (!targetLanguage.isEmpty() && tsFileNames.count() != 1)
        printErr(LU::tr("lupdate warning: -target-language usually only"
                        " makes sense with exactly one TS file.\n"));

    QString errorString;
    if (!proFiles.isEmpty()) {
        runQtTool(QStringLiteral("lupdate-pro"), app.arguments().mid(1));
        return 0;
    }

    Projects projectDescription;
    if (!projectDescriptionFile.isEmpty()) {
        projectDescription = readProjectDescription(projectDescriptionFile, &errorString);
        if (!errorString.isEmpty()) {
            printErr(LU::tr("lupdate error: %1\n").arg(errorString));
            return 1;
        }
        if (projectDescription.empty()) {
            printErr(LU::tr("lupdate error:"
                            " Could not find project descriptions in %1.\n")
                     .arg(projectDescriptionFile));
            return 1;
        }
    }

    bool fail = false;
    if (projectDescription.empty()) {
        if (tsFileNames.isEmpty())
            printErr(LU::tr("lupdate warning:"
                            " no TS files specified. Only diagnostics will be produced.\n"));

        Translator fetchedTor;
        ConversionData cd;
        cd.m_noUiLines = options & NoUiLines;
        cd.m_sourceIsUtf16 = options & SourceIsUtf16;
        cd.m_projectRoots = projectRoots;
        cd.m_includePath = includePath;
        cd.m_allCSources = allCSources;
        for (const QString &resource : qAsConst(resourceFiles))
            sourceFiles << getResources(resource);
        processSources(fetchedTor, sourceFiles, cd);
        updateTsFiles(fetchedTor, tsFileNames, alienFiles,
                      sourceLanguage, targetLanguage, options, &fail);
    } else {
        if (!sourceFiles.isEmpty() || !resourceFiles.isEmpty() || !includePath.isEmpty()) {
            printErr(LU::tr("lupdate error:"
                            " Both project and source files / include paths specified.\n"));
            return 1;
        }
        QString errorString;
        ProjectProcessor projectProcessor(sourceLanguage, targetLanguage);
        if (!tsFileNames.isEmpty()) {
            Translator fetchedTor;
            projectProcessor.processProjects(true, options, projectDescription, true, &fetchedTor,
                                             &fail);
            if (!fail) {
                updateTsFiles(fetchedTor, tsFileNames, alienFiles,
                              sourceLanguage, targetLanguage, options, &fail);
            }
        } else {
            projectProcessor.processProjects(true, options, projectDescription, false, nullptr,
                                             &fail);
        }
    }
    return fail ? 1 : 0;
}
