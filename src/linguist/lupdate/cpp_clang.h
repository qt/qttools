// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CLANG_CPP_H
#define CLANG_CPP_H

#include "lupdate.h"
#include "synchronized.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstring.h>

QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4100)
QT_WARNING_DISABLE_MSVC(4146)
QT_WARNING_DISABLE_MSVC(4267)
QT_WARNING_DISABLE_MSVC(4624)
QT_WARNING_DISABLE_GCC("-Wnonnull")

#include <llvm/ADT/StringRef.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/FileManager.h>

QT_WARNING_POP

#include <vector>
#include <iostream>
#include <sstream>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcClang)

inline QString toQt(llvm::StringRef str)
{
    return QString::fromUtf8(str.data(), str.size());
}

#define LUPDATE_CLANG_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define LUPDATE_CLANG_VERSION LUPDATE_CLANG_VERSION_CHECK(LUPDATE_CLANG_VERSION_MAJOR, \
    LUPDATE_CLANG_VERSION_MINOR, LUPDATE_CLANG_VERSION_PATCH)

// Local storage of translation information (information from the AST and linguist side)
struct TranslationRelatedStore
{
    QString callType;
    QString rawCode;
    QString funcName;
    qint64 locationCol = -1;
    QString contextArg;
    QString contextRetrieved;
    QString lupdateSource;
    QString lupdateLocationFile;
    QString lupdateInputFile; // file associated to the running of the tool
    qint64 lupdateLocationLine = -1;
    QString lupdateId;
    QString lupdateSourceWhenId;
    QString lupdateIdMetaData;
    QString lupdateMagicMetaData;
    QHash<QString, QString> lupdateAllMagicMetaData;
    QString lupdateComment;
    QString lupdateExtraComment;
    QString lupdatePlural;
    QString lupdateWarning;
    clang::SourceLocation sourceLocation;

    bool isValid(bool printwarning = false)
    {
        switch (trFunctionAliasManager.trFunctionByName(funcName)) {
        // only one argument: the source
        case TrFunctionAliasManager::Function_Q_DECLARE_TR_FUNCTIONS:
            if (contextArg.isEmpty()) {
                if (printwarning) {
                    std::stringstream warning;
                    warning << qPrintable(lupdateLocationFile) << ":"
                        << lupdateLocationLine << ":"
                        << locationCol << ": "
                        << " \'" << qPrintable(funcName)
                        << "\' cannot be called without context."
                        << " The call is ignored." <<std::endl;
                    lupdateWarning.append(QString::fromStdString(warning.str()));
                }
                return false;
            }
            break;
        case TrFunctionAliasManager::Function_tr:
        case TrFunctionAliasManager::Function_trUtf8:
            if (lupdateSource.isEmpty()) {
                if (printwarning) {
                    std::stringstream warning;
                    warning << qPrintable(lupdateLocationFile) << ":"
                        << lupdateLocationLine << ":"
                        << locationCol << ": "
                        << " \'" << qPrintable(funcName)
                        << "\' cannot be called without source."
                        << " The call is ignored." << std::endl;
                    lupdateWarning.append(QString::fromStdString(warning.str()));
                }
                return false;
            }
            break;
        // two arguments: the context and the source
        case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
        case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
        case TrFunctionAliasManager::Function_translate:
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
            if (contextArg.isEmpty() || lupdateSource.isEmpty()) {
                if (printwarning) {
                    std::stringstream warning;
                    warning << qPrintable(lupdateLocationFile) << ":"
                        << lupdateLocationLine << ":"
                        << locationCol << ": "
                        << " \'" << qPrintable(funcName)
                        << "\' cannot be called without context or source."
                        << " The call is ignored." << std::endl;
                    lupdateWarning.append(QString::fromStdString(warning.str()));
                }
                return false;
            }
            // not sure if the third argument is compulsory
            break;
        // only one argument (?) the message Id
        case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
        case TrFunctionAliasManager::Function_qtTrId:
        case TrFunctionAliasManager::Function_QT_TRID_NOOP:
            if (lupdateId.isEmpty()) {
                if (printwarning) {
                    std::stringstream warning;
                    warning << qPrintable(lupdateLocationFile) << ":"
                        << lupdateLocationLine << ":"
                        << locationCol << ": "
                        << " \'" << qPrintable(funcName)
                        << "\' cannot be called without Id."
                        << " The call is ignored." << std::endl;
                    lupdateWarning.append(QString::fromStdString(warning.str()));
                }
                return false;
            }
            break;
        default:
            if (funcName == QStringLiteral("TRANSLATOR") && lupdateComment.isEmpty()) {
                if (printwarning) {
                    std::stringstream warning;
                    warning << qPrintable(lupdateLocationFile) << ":"
                        << lupdateLocationLine << ":"
                        << locationCol << ": "
                        << qPrintable(funcName)
                        << " cannot be called without comment."
                        << " The call is ignored." << std::endl;
                    lupdateWarning.append(QString::fromStdString(warning.str()));
                }
                return false;
            }
        }
        return !lupdateLocationFile.isEmpty() && (lupdateLocationLine > -1) && (locationCol > -1);
    }

    clang::SourceLocation callLocation(const clang::SourceManager &sourceManager)
    {
        if (sourceLocation.isInvalid()) {
            auto sourceFile = sourceManager.getFileManager()
                .getFile(lupdateLocationFile.toStdString());
#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
            sourceLocation = sourceManager.translateFileLineCol(sourceFile.get(),
                lupdateLocationLine, locationCol);
#else
            sourceLocation = sourceManager.translateFileLineCol(sourceFile, lupdateLocationLine,
                locationCol);
#endif
        }
        return sourceLocation;
    }

    void printStore() const
    {
        qCDebug(lcClang) << "------------------ Printing Store----------------------------------\n";
        qCDebug(lcClang)
            << "callType            : " << callType << "\n"
            << "rawCode             : \n" << rawCode << "\n"
            << "funcName            : " << funcName << "\n"
            << "LocationCol         : " << locationCol << "\n"
            << "contextArg          : " << contextArg << "\n"
            << "contextRetrieved    : " << contextRetrieved << "\n"
            << "lupdateSource       : " << lupdateSource << "\n"
            << "lupdateLocationFile : " << lupdateLocationFile << "\n"
            << "lupdateLocationLine : " << lupdateLocationLine << "\n"
            << "lupdateId           : " << lupdateId << "\n"
            << "lupdateIdMetaData   : " << lupdateIdMetaData << "\n"
            << "lupdateMagicMetaData: " << lupdateMagicMetaData << "\n"
            << "lupdateComment      : " << lupdateComment << "\n"
            << "lupdateExtraComment : " << lupdateExtraComment << "\n"
            << "lupdatePlural       : " << lupdatePlural;
        qCDebug(lcClang) << "-------------------------------------------------------------------\n";
    }
};
using TranslationStores = std::vector<TranslationRelatedStore>;

struct Stores
{
    Stores(TranslationStores &a, TranslationStores &qd, TranslationStores &qn)
        : AST(a)
        , QDeclareTrWithContext(qd)
        , QNoopTranlsationWithContext(qn)
    {}

    TranslationStores Preprocessor;
    WriteSynchronizedRef<TranslationRelatedStore> AST;
    WriteSynchronizedRef<TranslationRelatedStore> QDeclareTrWithContext;
    WriteSynchronizedRef<TranslationRelatedStore> QNoopTranlsationWithContext; // or with warnings that need to be
                                                                               //displayed in the same order, always
};

namespace LupdatePrivate
{
    inline QString fixedLineEndings(const QString &s)
    {
#ifdef Q_OS_WIN
        QString result = s;
        result.replace(QLatin1String("\r\n"), QLatin1String("\n"));
        return result;
#else
        return s;
#endif
    }

    enum QuoteCompulsary
    {
        None = 0x01,
        Left = 0x02,                // Left quote is mandatory
        Right = 0x04,               // Right quote is mandatory
        LeftAndRight = Left | Right // Both quotes are mandatory
    };

    /*
        Removes the quotes around the lupdate extra, ID meta data, magic and
        ID prefix comments and source string literals.
        Depending on the given compulsory option, quotes can be unbalanced and
        still some text is returned. This is to mimic the old lupdate behavior.
    */
    inline QString cleanQuote(llvm::StringRef s, QuoteCompulsary quote)
    {
        if (s.empty())
            return {};
        s = s.trim();
        if (!s.consume_front("\"") && ((quote & Left) != 0))
            return {};
        if (!s.consume_back("\"") && ((quote & Right) != 0))
            return {};
        return fixedLineEndings(toQt(s));
    }

    /*
        Removes the quotes and a possible existing string literal prefix
        for a given string literal coming from the source code. Do not use
        to clean the quotes around the lupdate translator specific comments.
    */
    inline QString cleanQuote(const std::string &token)
    {
        if (token.empty())
            return {};

        const QString string = fixedLineEndings(QString::fromStdString(token).trimmed());
        const int index = string.indexOf(QLatin1Char('"'));
        if (index <= 0)
            return LupdatePrivate::cleanQuote(token, QuoteCompulsary::LeftAndRight);

        QRegularExpressionMatch result;
        if (string.at(index - 1) == QLatin1Char('R')) {
            static const QRegularExpression rawStringLiteral {
                QStringLiteral(
                    "(?:\\bu8|\\b[LuU])??R\\\"([^\\(\\)\\\\ ]{0,16})\\((?<characters>.*)\\)\\1\\\""
                ), QRegularExpression::DotMatchesEverythingOption };
            result = rawStringLiteral.match(string);
        } else {
            static const QRegularExpression stringLiteral {
                QStringLiteral(
                    "(?:\\bu8|\\b[LuU])+?\\\"(?<characters>[^\\\"\\\\]*(?:\\\\.[^\\\"\\\\]*)*)\\\""
                )
            };
            result = stringLiteral.match(string);
        }
        if (result.hasMatch())
            return result.captured(QStringLiteral("characters"));
        return string;
    }
}

namespace ClangCppParser
{
    void loadCPP(Translator &translator, const QStringList &filenames, ConversionData &cd,
                 bool *fail);

    using TranslatorMessageVector = std::vector<TranslatorMessage>;
    void collectMessages(TranslatorMessageVector &result, TranslationRelatedStore &store);
    TranslatorMessage translatorMessage(const TranslationRelatedStore &store,
        const QString &id, bool plural, bool isID, bool isWarningOnly = false);

    void correctAstTranslationContext(ReadSynchronizedRef<TranslationRelatedStore> &ast,
        WriteSynchronizedRef<TranslationRelatedStore> &newAst, const TranslationStores &qDecl);
    void finalize(ReadSynchronizedRef<TranslationRelatedStore> &ast,
        WriteSynchronizedRef<TranslationRelatedStore> &newAst);

    bool stringContainsTranslationInformation(llvm::StringRef ba);
    bool hasAliases();
    std::vector<std::string> getAliasFunctionDefinition();

}

QT_END_NAMESPACE

#endif
