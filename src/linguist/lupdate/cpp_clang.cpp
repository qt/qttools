/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "cpp_clang.h"
#include "clangtoolastreader.h"
#include "lupdatepreprocessoraction.h"
#include "translator.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Option/Option.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcClang, "qt.lupdate.clang");

// This is a way to add options related to the customized clang tool
// Needed as one of the arguments to create the OptionParser.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// Makes sure all the comments will be parsed and part of the AST
// Clang will run with the flag -fparse-all-comments
clang::tooling::ArgumentsAdjuster getClangArgumentAdjuster()
{
    return [](const clang::tooling::CommandLineArguments &args, llvm::StringRef /*unused*/) {
        clang::tooling::CommandLineArguments adjustedArgs;
        for (size_t i = 0, e = args.size(); i < e; ++i) {
            llvm::StringRef arg = args[i];
            // FIXME: Remove options that generate output.
            if (!arg.startswith("-fcolor-diagnostics") && !arg.startswith("-fdiagnostics-color"))
                adjustedArgs.push_back(args[i]);
        }
        adjustedArgs.push_back("-fparse-all-comments");
        adjustedArgs.push_back("-I");
        adjustedArgs.push_back(CLANG_RESOURCE_DIR);
        return adjustedArgs;
    };
}

void ClangCppParser::loadCPP(Translator &translator, const QStringList &filenames,
    ConversionData &cd)
{
    std::vector<std::string> sources;
    for (const QString &filename : filenames)
        sources.push_back(filename.toStdString());

    int argc = 4;
    // NEED 2 empty one to start!!! otherwise: LLVM::ERROR
    const QByteArray jsonPath = cd.m_compileCommandsPath.toLocal8Bit();
    const char *argv[4] = { "", "", "-p", jsonPath.constData() };
    clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

    clang::tooling::ClangTool tool(OptionsParser.getCompilations(), sources);
    tool.appendArgumentsAdjuster(getClangArgumentAdjuster());

    Stores stores;
    tool.run(new LupdatePreprocessorActionFactory(stores.Preprocessor));
    tool.run(new LupdateToolActionFactory(stores));

    Translator *tor = new Translator();
    ClangCppParser::fillTranslator(tor, stores);

    if (QLoggingCategory("qt.lupdate.clang").isDebugEnabled())
        tor->dump();

    for (const TranslatorMessage &msg : tor->messages())
        translator.extend(msg, cd);
}

/*
    Fill the Translator with the retrieved information after traversing the AST.
*/
void ClangCppParser::fillTranslator(Translator *tor, Stores &stores)
{
    correctAstTranslationContext(stores);
    for (auto &store : stores.AST)
        fillTranslator(tor, store);

    correctNoopTanslationContext(stores);
    for (auto &store : stores.QNoopTranlsationWithContext)
        fillTranslator(tor, store);
}

void ClangCppParser::fillTranslator(Translator *tor, TranslationRelatedStore store)
{
    bool plural = false;
    switch (trFunctionAliasManager.trFunctionByName(store.funcName)) {
    case TrFunctionAliasManager::Function_Q_DECLARE_TR_FUNCTIONS:
        break;
    case TrFunctionAliasManager::Function_QT_TR_N_NOOP:
        plural = true;
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_tr:
    case TrFunctionAliasManager::Function_trUtf8:
    case TrFunctionAliasManager::Function_QT_TR_NOOP:
    case TrFunctionAliasManager::Function_QT_TR_NOOP_UTF8:
        handleTr(tor, store, plural);
        break;
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
        plural = true;
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_translate:
    case TrFunctionAliasManager::Function_findMessage:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
        handleTranslate(tor, store, plural);
        break;
    case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
        plural = true;
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_qtTrId:
    case TrFunctionAliasManager::Function_QT_TRID_NOOP:
        handleTrId(tor, store, plural);
        break;
    default:
        if (store.funcName == QStringLiteral("TRANSLATOR"))
            handleTr(tor, store, false);
    }
}

TranslatorMessage ClangCppParser::fillTranslatorMessage(const TranslationRelatedStore &store,
    const QString &id, bool plural, bool isId)
{
    QString context;
    if (!isId) {
        context = ParserTool::transcode(store.contextArg.isEmpty() ? store.contextRetrieved
            : store.contextArg);
    }

    TranslatorMessage msg(context,
        ParserTool::transcode(isId ? store.lupdateSourceWhenId
            : store.lupdateSource),
        ParserTool::transcode(store.lupdateComment),
        QString(),
        store.lupdateLocationFile,
        store.lupdateLocationLine,
        QStringList(),
        TranslatorMessage::Type::Unfinished,
        (plural ? plural : !store.lupdatePlural.isEmpty()));

    if (!store.lupdateAllMagicMetaData.empty())
        msg.setExtras(store.lupdateAllMagicMetaData);
    msg.setExtraComment(ParserTool::transcode(store.lupdateExtraComment));
    msg.setId(ParserTool::transcode(id));
    return msg;
}

void ClangCppParser::handleTranslate(Translator *tor, const TranslationRelatedStore &store,
    bool plural)
{
    if (!store.lupdateSourceWhenId.isEmpty())
        qCDebug(lcClang) << "//% is ignored when using translate function\n";
    tor->append(fillTranslatorMessage(store, store.lupdateIdMetaData, plural, false));
}

void ClangCppParser::handleTr(Translator *tor, const TranslationRelatedStore &store, bool plural)
{
    if (!store.lupdateSourceWhenId.isEmpty())
        qCDebug(lcClang) << "//% is ignored when using tr function\n";
    if (store.contextRetrieved.isEmpty() && store.contextArg.isEmpty()) {
        qCDebug(lcClang) << "tr() cannot be called without context \n";
        return;
    }
    tor->append(fillTranslatorMessage(store, store.lupdateIdMetaData, plural, false));
}

void ClangCppParser::handleTrId(Translator *tor, const TranslationRelatedStore &store, bool plural)
{
    if (!store.lupdateIdMetaData.isEmpty())
        qCDebug(lcClang) << "//= is ignored when using qtTrId function \n";
    tor->append(fillTranslatorMessage(store, store.lupdateId, plural, true));
}

void ClangCppParser::correctAstTranslationContext(Stores &stores)
{
    for (auto &store : stores.AST) {
        if (!store.contextArg.isEmpty())
            continue;

        // If there is a Q_DECLARE_TR_FUNCTION the context given there takes
        // priority over the retrieved context. The retrieved context for
        // Q_DECLARE_TR_FUNCTION (where the macro was) has to fit the retrieved
        // context of the tr function if there is already a argument giving the
        // context, it has priority
        for (auto &declareStore : stores.QDeclareTrWithContext) {
            qCDebug(lcClang) << "----------------------------";
            qCDebug(lcClang) << "Tr call context retrieved " << store.contextRetrieved;
            qCDebug(lcClang) << "Tr call source            " << store.lupdateSource;
            qCDebug(lcClang) << "- DECLARE context retrieved " << declareStore.contextRetrieved;
            qCDebug(lcClang) << "- DECLARE context Arg       " << declareStore.contextArg;
            if (declareStore.contextRetrieved.isEmpty())
                continue;
            if (!declareStore.contextRetrieved.startsWith(store.contextRetrieved))
                continue;
            if (store.contextRetrieved.size() == declareStore.contextRetrieved.size()) {
                qCDebug(lcClang) << "* Tr call context retrieved " << store.contextRetrieved;
                qCDebug(lcClang) << "* DECLARE context retrieved " << declareStore.contextRetrieved;
                qCDebug(lcClang) << "* DECLARE context Arg       " << declareStore.contextArg;
                store.contextArg = declareStore.contextArg;
            }
        }
    }
}

void ClangCppParser::correctNoopTanslationContext(Stores &stores)
{
    for (auto &store : stores.QNoopTranlsationWithContext) {
        if (!store.contextArg.isEmpty())
            continue;
        qCDebug(lcClang) << "----------------------------";
        qCDebug(lcClang) << "NOOP call context retrieved Temp" << store.contextRetrievedTempNOOP;
        qCDebug(lcClang) << "NOOP call source            " << store.lupdateSource;

        for (const auto &qDeclare : stores.QDeclareTrWithContext) {
            bool firstCheck = false;
            bool secondCheck = false;
            qCDebug(lcClang) << "- DECLARE context retrieved " << qDeclare.contextRetrieved;
            qCDebug(lcClang) << "- DECLARE context Arg       " << qDeclare.contextArg;
            if (store.contextRetrievedTempNOOP.startsWith(qDeclare.contextRetrieved)) {
                firstCheck = (store.contextRetrievedTempNOOP.size() == qDeclare.contextRetrieved.size()
                    || (store.contextRetrievedTempNOOP.at(qDeclare.contextRetrieved.size() + 1)
                        == QLatin1Char(':')));
                secondCheck = qDeclare.contextRetrieved.size() > store.contextRetrieved.size();
                if (firstCheck && secondCheck) {
                    store.contextRetrieved = qDeclare.contextRetrieved;
                    store.contextArg = qDeclare.contextArg;
                    qCDebug(lcClang) << "* NOOP call context retrieved " << store.contextRetrieved;
                    qCDebug(lcClang) << "* DECLARE context retrieved   " << qDeclare.contextRetrieved;
                    qCDebug(lcClang) << "* DECLARE context Arg         " << qDeclare.contextArg;
                }
            }
        }
    }
}

QT_END_NAMESPACE
