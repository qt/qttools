// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "trparser.h"
#include "translator.h"
#include <iostream>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

static bool processTs(Translator &fetchedTor, const QString &file, ConversionData &cd)
{
    for (const Translator::FileFormat &fmt : std::as_const(Translator::registeredFileFormats())) {
        if (file.endsWith(u'.' + fmt.extension, Qt::CaseInsensitive)) {
            Translator tor;
            if (tor.load(file, cd, fmt.extension)) {
                for (TranslatorMessage msg : tor.messages()) {
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

void processSources(Translator &fetchedTor, const QStringList &sourceFiles, ConversionData &cd)
{
#ifdef QT_NO_QML
    bool requireQmlSupport = false;
#endif
    QStringList sourceFilesCpp;
    for (const auto &sourceFile : sourceFiles) {
        if (sourceFile.endsWith(".java"_L1, Qt::CaseInsensitive))
            loadJava(fetchedTor, sourceFile, cd);
        else if (sourceFile.endsWith(".ui"_L1, Qt::CaseInsensitive)
                 || sourceFile.endsWith(".jui"_L1, Qt::CaseInsensitive))
            loadUI(fetchedTor, sourceFile, cd);
#ifndef QT_NO_QML
        else if (sourceFile.endsWith(".js"_L1, Qt::CaseInsensitive)
                 || sourceFile.endsWith(".qs"_L1, Qt::CaseInsensitive)) {
            loadQScript(fetchedTor, sourceFile, cd);
        } else if (sourceFile.endsWith(".mjs"_L1, Qt::CaseInsensitive)) {
            loadJSModule(fetchedTor, sourceFile, cd);
        } else if (sourceFile.endsWith(".qml"_L1, Qt::CaseInsensitive))
            loadQml(fetchedTor, sourceFile, cd);
#else
        else if (sourceFile.endsWith(".qml"_L1, Qt::CaseInsensitive)
                 || sourceFile.endsWith(".js"_L1, Qt::CaseInsensitive)
                 || sourceFile.endsWith(".mjs"_L1, Qt::CaseInsensitive)
                 || sourceFile.endsWith(".qs"_L1, Qt::CaseInsensitive))
            requireQmlSupport = true;
#endif // QT_NO_QML
        else if (sourceFile.endsWith(u".py", Qt::CaseInsensitive))
            loadPython(fetchedTor, sourceFile, cd);
        else if (!processTs(fetchedTor, sourceFile, cd))
            sourceFilesCpp << sourceFile;
    }

#ifdef QT_NO_QML
    if (requireQmlSupport) {
        std::cout << qPrintable("missing qml/javascript support\n"
                                "Some files have been ignored.\n"_L1);
    }
#endif

    loadCPP(fetchedTor, sourceFilesCpp, cd);

    if (!cd.error().isEmpty())
        std::cerr << qPrintable(cd.error());
}

QString transcode(const QString &str)
{
    static const char tab[] = "abfnrtv";
    static const char backTab[] = "\a\b\f\n\r\t\v";
    // This function has to convert back to bytes, as C's \0* sequences work at that level.
    const QByteArray in = str.toUtf8();
    QByteArray out;

    out.reserve(in.size());
    for (qsizetype i = 0; i < in.size();) {
        uchar c = in[i++];
        if (c == '\\') {
            if (i >= in.size())
                break;
            c = in[i++];

            if (c == '\n')
                continue;

            if (c == 'x' || c == 'u' || c == 'U') {
                qsizetype maxSize = 2; // \x
                if (c == 'u')
                    maxSize = 4;
                else if (c == 'U')
                    maxSize = 8;
                maxSize = std::min(in.size(), i + maxSize);

                const bool unicode = (c != 'x');
                QByteArray hex;
                while (i < maxSize && isxdigit((c = in[i]))) {
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
                while (n < 2 && i < in.size() && (c = in[i]) >= '0' && c < '8') {
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
    return QString::fromUtf8(out.constData(), out.size());
}

// Can't have an array of QStaticStringData<N> for different N, so
// use QString, which requires constructor calls. Doesn't matter
// much, since this is in an app, not a lib:
static const QString defaultTrFunctionNames[] = {
// MSVC can't handle the lambda in this array if QStringLiteral expands
// to a lambda. In that case, use a QString instead.
#if defined(Q_CC_MSVC) && defined(Q_COMPILER_LAMBDA)
#  define STRINGLITERAL(F) QLatin1String(#F),
#else
#  define STRINGLITERAL(F) QStringLiteral(#F),
#endif
    LUPDATE_FOR_EACH_TR_FUNCTION(STRINGLITERAL)
#undef STRINGLITERAL
};

Q_STATIC_ASSERT((TrFunctionAliasManager::NumTrFunctions
                 == sizeof defaultTrFunctionNames / sizeof *defaultTrFunctionNames));

TrFunctionAliasManager trFunctionAliasManager;

QStringList availableFunctions()
{
    QStringList result;
    result.reserve(TrFunctionAliasManager::NumTrFunctions);
    for (int i = 0; i < TrFunctionAliasManager::NumTrFunctions; ++i)
        result.push_back(defaultTrFunctionNames[i]);
    return result;
}

int trFunctionByDefaultName(const QString &trFunctionName)
{
    for (int i = 0; i < TrFunctionAliasManager::NumTrFunctions; ++i)
        if (trFunctionName == defaultTrFunctionNames[i])
            return i;
    return -1;
}

static void printErr(const QString &out)
{
    std::cerr << qPrintable(out);
}

bool parseTrFunctionAliasString(const QString &aliasString)
{
    for (const QString &pair : aliasString.split(u',', Qt::SkipEmptyParts)) {
        const int equalSign = pair.indexOf(u'=');
        if (equalSign < 0) {
            printErr("tr-function mapping '%1' in -tr-function-alias is missing the '='.\n"_L1.arg(
                    pair));
            return false;
        }
        const bool plusEqual = equalSign > 0 && pair[equalSign - 1] == u'+';
        const int trFunctionEnd = plusEqual ? equalSign - 1 : equalSign;
        const QString trFunctionName = pair.left(trFunctionEnd).trimmed();
        const QString alias = pair.mid(equalSign + 1).trimmed();
        const int trFunction = trFunctionByDefaultName(trFunctionName);
        if (trFunction < 0) {
            printErr("Unknown tr-function '%1' in -tr-function-alias option.\n"
                     "Available tr-functions are: %2\n"_L1.arg(trFunctionName,
                                                               availableFunctions().join(u',')));
            return false;
        }
        if (alias.isEmpty()) {
            printErr("Empty alias for tr-function '%1' in -tr-function-alias option.\n"_L1.arg(
                    trFunctionName));
            return false;
        }
        trFunctionAliasManager.modifyAlias(trFunction, alias,
                                           plusEqual ? TrFunctionAliasManager::AddAlias
                                                     : TrFunctionAliasManager::SetAlias);
    }
    return true;
}

TrFunctionAliasManager::TrFunctionAliasManager() : m_trFunctionAliases()
{
    for (int i = 0; i < NumTrFunctions; ++i)
        m_trFunctionAliases[i].push_back(defaultTrFunctionNames[i]);
}

TrFunctionAliasManager::~TrFunctionAliasManager() { }

int TrFunctionAliasManager::trFunctionByName(const QString &trFunctionName) const
{
    ensureTrFunctionHashUpdated();
    // this function needs to be fast
    const auto it = m_nameToTrFunctionMap.constFind(trFunctionName);
    return it == m_nameToTrFunctionMap.cend() ? -1 : *it;
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

    NameToTrFunctionMap nameToTrFunctionMap;
    for (int i = 0; i < NumTrFunctions; ++i)
        for (const QString &alias : m_trFunctionAliases[i])
            nameToTrFunctionMap[alias] = TrFunction(i);
    // commit:
    m_nameToTrFunctionMap.swap(nameToTrFunctionMap);
}

const TrFunctionAliasManager::NameToTrFunctionMap &
TrFunctionAliasManager::nameToTrFunctionMap() const
{
    ensureTrFunctionHashUpdated();
    return m_nameToTrFunctionMap;
}

QStringList TrFunctionAliasManager::availableFunctionsWithAliases() const
{
    QStringList result;
    result.reserve(NumTrFunctions);
    for (int i = 0; i < NumTrFunctions; ++i)
        result.push_back(defaultTrFunctionNames[i] + " (="_L1 + m_trFunctionAliases[i].join(u'=')
                         + u')');
    return result;
}

QStringList TrFunctionAliasManager::listAliases() const
{
    QStringList result;
    result.reserve(NumTrFunctions);
    for (int i = 0; i < NumTrFunctions; ++i) {
        for (int ii = 1; ii < m_trFunctionAliases[i].size(); ii++) {
            // ii = 0 is the default name. Not listed here
            result.push_back(m_trFunctionAliases[i][ii]);
        }
    }
    return result;
}

QT_END_NAMESPACE
