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
#include "synchronized.h"
#include "translator.h"

#include <QtCore/qthread.h>

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
        adjustedArgs.push_back("-fsyntax-only");
        return adjustedArgs;
    };
}

void ClangCppParser::loadCPP(Translator &translator, const QStringList &files, ConversionData &cd)
{
    // pre-process the files by a simple text search if there is any occurrence
    // of things we are interested in
    constexpr llvm::StringLiteral qDeclareTrFunction("Q_DECLARE_TR_FUNCTIONS(");
    constexpr llvm::StringLiteral qtTrNoop("QT_TR_NOOP(");
    constexpr llvm::StringLiteral qtTrNoopUTF8("QT_TR_NOOP)UTF8(");
    constexpr llvm::StringLiteral qtTrNNoop("QT_TR_N_NOOP(");
    constexpr llvm::StringLiteral qtTrIdNoop("QT_TRID_NOOP(");
    constexpr llvm::StringLiteral qtTrIdNNoop("QT_TRID_N_NOOP(");
    constexpr llvm::StringLiteral qtTranslateNoop("QT_TRANSLATE_NOOP(");
    constexpr llvm::StringLiteral qtTranslateNoopUTF8("QT_TRANSLATE_NOOP_UTF8(");
    constexpr llvm::StringLiteral qtTranslateNNoop("QT_TRANSLATE_N_NOOP(");
    constexpr llvm::StringLiteral qtTranslateNoop3("QT_TRANSLATE_NOOP3(");
    constexpr llvm::StringLiteral qtTranslateNoop3UTF8("QT_TRANSLATE_NOOP3_UTF8(");
    constexpr llvm::StringLiteral qtTranslateNNoop3("QT_TRANSLATE_N_NOOP3(");
    constexpr llvm::StringLiteral translatorComment("TRANSLATOR ");
    constexpr llvm::StringLiteral qtTrId("qtTrId(");
    constexpr llvm::StringLiteral tr("tr(");
    constexpr llvm::StringLiteral trUtf8("trUtf8(");
    constexpr llvm::StringLiteral translate("translate(");

    std::vector<std::string> sources, sourcesAst, sourcesPP;
    for (const QString &filename : files) {
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            if (const uchar *memory = file.map(0, file.size())) {
                const auto ba = llvm::StringRef((const char*) (memory), file.size());
                if (ba.contains(qDeclareTrFunction)) {
                    sourcesPP.emplace_back(filename.toStdString());
                    sourcesAst.emplace_back(sourcesPP.back());
                    continue;
                }
                const size_t pos = ba.find_first_of("QT_TR");
                const auto baSliced = ba.slice(pos, llvm::StringRef::npos);
                 if (pos != llvm::StringRef::npos) {
                     if (baSliced.contains(qtTrNoop) || baSliced.contains(qtTrNoopUTF8) || baSliced.contains(qtTrNNoop)
                         || baSliced.contains(qtTrIdNoop) || baSliced.contains(qtTrIdNNoop)
                         || baSliced.contains(qtTranslateNoop) ||  baSliced.contains(qtTranslateNoopUTF8)
                         ||  baSliced.contains(qtTranslateNNoop) || baSliced.contains(qtTranslateNoop3)
                         ||  baSliced.contains(qtTranslateNoop3UTF8) ||  baSliced.contains(qtTranslateNNoop3)) {
                         sourcesPP.emplace_back(filename.toStdString());
                         sourcesAst.emplace_back(sourcesPP.back());
                         continue;
                     }
                 }
                if (ba.contains(translatorComment) || ba.contains(qtTrId) || ba.contains(tr)
                    || ba.contains(trUtf8) || ba.contains(translate)) {
                    sourcesAst.emplace_back(filename.toStdString());
                }
            } else {
                // mmap did not succeed, remember it anyway
                sources.push_back(filename.toStdString());
            }
        } else {
            // we could not open the file, remember it anyway
            sources.push_back(filename.toStdString());
        }
    }
    sourcesPP.insert(sourcesPP.end(), sources.begin(), sources.end());
    sourcesAst.insert(sourcesAst.end(), sources.begin(), sources.end());

    int argc = 4;
    // NEED 2 empty one to start!!! otherwise: LLVM::ERROR
    const QByteArray jsonPath = cd.m_compileCommandsPath.toLocal8Bit();
    const char *argv[4] = { "", "", "-p", jsonPath.constData() };
    clang::tooling::CommonOptionsParser optionsParser(argc, argv, MyToolCategory);

    TranslationStores ast, qdecl, qnoop;
    Stores stores(ast, qdecl, qnoop);

    std::vector<std::thread> producers;
    ReadSynchronizedRef<std::string> ppSources(sourcesPP);
    WriteSynchronizedRef<TranslationRelatedStore> ppStore(stores.Preprocessor);
    int idealProducerCount = std::min(int(sourcesPP.size()), QThread::idealThreadCount());

    for (int i = 0; i < idealProducerCount; ++i) {
        std::thread producer([&ppSources, &optionsParser, &ppStore]() {
            std::string file;
            while (ppSources.next(&file)) {
                clang::tooling::ClangTool tool(optionsParser.getCompilations(), file);
                tool.appendArgumentsAdjuster(getClangArgumentAdjuster());
                tool.run(new LupdatePreprocessorActionFactory(&ppStore));
            }
        });
        producers.emplace_back(std::move(producer));
    }
    for (auto &producer : producers)
        producer.join();
    producers.clear();

    ReadSynchronizedRef<std::string> astSources(sourcesAst);
    idealProducerCount = std::min(int(sourcesAst.size()), QThread::idealThreadCount());
    for (int i = 0; i < idealProducerCount; ++i) {
        std::thread producer([&astSources, &optionsParser, &stores]() {
            std::string file;
            while (astSources.next(&file)) {
                clang::tooling::ClangTool tool(optionsParser.getCompilations(), file);
                tool.appendArgumentsAdjuster(getClangArgumentAdjuster());
                tool.run(new LupdateToolActionFactory(&stores));
            }
        });
        producers.emplace_back(std::move(producer));
    }
    for (auto &producer : producers)
        producer.join();
    producers.clear();

    TranslationStores finalStores;
    WriteSynchronizedRef<TranslationRelatedStore> wsv(finalStores);

    ReadSynchronizedRef<TranslationRelatedStore> rsv(ast);
    ClangCppParser::correctAstTranslationContext(rsv, wsv, qdecl);

    rsv.reset(qnoop);
    ClangCppParser::correctNoopTanslationContext(rsv, wsv, qdecl);

    for (const auto &store : finalStores)
        ClangCppParser::fillTranslator(store, translator, cd);
}

void ClangCppParser::fillTranslator(const TranslationRelatedStore &store, Translator &translator,
    ConversionData &cd)
{
    if (!store.isValid())
        return;

    bool plural = false;
    switch (trFunctionAliasManager.trFunctionByName(store.funcName)) {
    // handle tr
    case TrFunctionAliasManager::Function_QT_TR_N_NOOP:
        plural = true;
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_tr:
    case TrFunctionAliasManager::Function_trUtf8:
    case TrFunctionAliasManager::Function_QT_TR_NOOP:
    case TrFunctionAliasManager::Function_QT_TR_NOOP_UTF8:
        if (!store.lupdateSourceWhenId.isEmpty())
            qCDebug(lcClang) << "//% is ignored when using tr function\n";
        if (store.contextRetrieved.isEmpty() && store.contextArg.isEmpty())
            qCDebug(lcClang) << "tr() cannot be called without context \n";
        else
            translator.extend(translatorMessage(store, store.lupdateIdMetaData, plural, false), cd);
        break;

    // handle translate and findMessage
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
        if (!store.lupdateSourceWhenId.isEmpty())
            qCDebug(lcClang) << "//% is ignored when using translate function\n";
        translator.extend(translatorMessage(store, store.lupdateIdMetaData, plural, false), cd);
        break;

    // handle qtTrId
    case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
        plural = true;
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_qtTrId:
    case TrFunctionAliasManager::Function_QT_TRID_NOOP:
        if (!store.lupdateIdMetaData.isEmpty())
            qCDebug(lcClang) << "//= is ignored when using qtTrId function \n";
        translator.extend(translatorMessage(store, store.lupdateId, plural, true), cd);
        break;
    default:
        if (store.funcName == QStringLiteral("TRANSLATOR"))
             translator.extend(translatorMessage(store, store.lupdateIdMetaData, plural, false), cd);
    }
}

TranslatorMessage ClangCppParser::translatorMessage(const TranslationRelatedStore &store,
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

#define START_THREADS(RSV, WSV) \
    std::vector<std::thread> producers; \
    const int idealProducerCount = std::min(RSV.size(), QThread::idealThreadCount()); \
    \
    for (int i = 0; i < idealProducerCount; ++i) { \
        std::thread producer([&]() { \
            TranslationRelatedStore store; \
            while (RSV.next(&store)) { \
                if (!store.contextArg.isEmpty()) { \
                    WSV.emplace_back(std::move(store)); \
                    continue; \
                }

#define JOIN_THREADS(WSV) \
                WSV.emplace_back(std::move(store)); \
            } \
        }); \
        producers.emplace_back(std::move(producer)); \
    } \
    \
    for (auto &producer : producers) \
        producer.join();

void ClangCppParser::correctAstTranslationContext(ReadSynchronizedRef<TranslationRelatedStore> &ast,
    WriteSynchronizedRef<TranslationRelatedStore> &newAst, const TranslationStores &qDecl)
{
    START_THREADS(ast, newAst)

    // If there is a Q_DECLARE_TR_FUNCTION the context given there takes
    // priority over the retrieved context. The retrieved context for
    // Q_DECLARE_TR_FUNCTION (where the macro was) has to fit the retrieved
    // context of the tr function if there is already a argument giving the
    // context, it has priority
    for (const auto &declareStore : qDecl) {
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

    JOIN_THREADS(newAst)
}

void ClangCppParser::correctNoopTanslationContext(ReadSynchronizedRef<TranslationRelatedStore> &qNoop,
    WriteSynchronizedRef<TranslationRelatedStore> &newQNoop, const TranslationStores &qDecl)
{
    START_THREADS(qNoop, newQNoop)

    qCDebug(lcClang) << "----------------------------";
    qCDebug(lcClang) << "NOOP call context retrieved Temp" << store.contextRetrievedTempNOOP;
    qCDebug(lcClang) << "NOOP call source            " << store.lupdateSource;

    for (const auto &qDeclare : qDecl) {
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

    JOIN_THREADS(newQNoop)
}

#undef START_THREADS
#undef JOIN_THREADS

QT_END_NAMESPACE
