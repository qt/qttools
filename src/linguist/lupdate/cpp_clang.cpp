// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cpp_clang.h"
#include "clangtoolastreader.h"
#include "filesignificancecheck.h"
#include "lupdatepreprocessoraction.h"
#include "synchronized.h"
#include "translator.h"

#include <QLibraryInfo>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/QProcess>
#include <QStandardPaths>
#include <QtTools/private/qttools-config_p.h>

#include <clang/Tooling/CompilationDatabase.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <iostream>
#include <cstdlib>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using clang::tooling::CompilationDatabase;

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

Q_LOGGING_CATEGORY(lcClang, "qt.lupdate.clang");

static QString getSysCompiler()
{
    QStringList candidates;
    if (const char* local_compiler = std::getenv("CXX")) {
        candidates.push_back(QLatin1String(local_compiler));
    } else {
        candidates = {
#ifdef Q_OS_WIN
            QStringLiteral("cl"),
#endif
            QStringLiteral("clang++"),
            QStringLiteral("gcc")
        };
    }
    QString sysCompiler;
    for (const QString &comp : candidates) {

        sysCompiler = QStandardPaths::findExecutable(comp);
        if (!sysCompiler.isEmpty())
            break;
    }
    return sysCompiler;
}

static QByteArrayList getMSVCIncludePathsFromEnvironment()
{
    QList<QByteArray> pathList;
    if (const char* includeEnv = std::getenv("INCLUDE")) {
        QByteArray includeList = QByteArray::fromRawData(includeEnv, strlen(includeEnv));
        pathList = includeList.split(';');
    }
    for (auto it = pathList.begin(); it != pathList.end(); ++it) {
        it->prepend("-isystem");
    }
    return pathList;
}

static QStringList getProjectDirsFromEnvironment()
{
    QList<QByteArray> dirList;
    QStringList rootdirs;
    if (const char* includeEnv = std::getenv("LUPDATE_ROOT_DIRS")) {
        QByteArray includeList = QByteArray::fromRawData(includeEnv, strlen(includeEnv));
        dirList = includeList.split(';');

        for (auto dir : dirList) {
            rootdirs.append(QString::fromStdString(dir.toStdString()));
        }
    }
    return rootdirs;
}


static QByteArray frameworkSuffix()
{
    return QByteArrayLiteral(" (framework directory)");
}

QByteArrayList getIncludePathsFromCompiler()
{

    QList<QByteArray> pathList;
    QString compiler = getSysCompiler();
    if (compiler.isEmpty()) {
        qWarning("lupdate: Could not determine system compiler.");
        return pathList;
    }

    const QFileInfo fiCompiler(compiler);
    const QString compilerName

#ifdef Q_OS_WIN
        = fiCompiler.completeBaseName();
#else
        = fiCompiler.fileName();
#endif

    if (compilerName == QLatin1String("cl"))
        return getMSVCIncludePathsFromEnvironment();

    if (compilerName != QLatin1String("gcc") && compilerName != QLatin1String("clang++")) {
        qWarning("lupdate: Unknown compiler %s", qPrintable(compiler));
        return pathList;
    }

    const QStringList compilerFlags = {
        QStringLiteral("-E"), QStringLiteral("-x"), QStringLiteral("c++"),
        QStringLiteral("-"), QStringLiteral("-v")
    };

    QProcess proc;
    proc.setStandardInputFile(proc.nullDevice());
    proc.start(compiler, compilerFlags);
    proc.waitForFinished(30000);
    QByteArray buffer = proc.readAllStandardError();
    proc.kill();

    // ### TODO: Merge this with qdoc's getInternalIncludePaths()
    const QByteArrayList stdErrLines = buffer.split('\n');
    bool isIncludeDir = false;
    for (const QByteArray &line : stdErrLines) {
        if (isIncludeDir) {
            if (line.startsWith(QByteArrayLiteral("End of search list"))) {
                isIncludeDir = false;
            } else {
                QByteArray prefix("-isystem");
                QByteArray headerPath{line.trimmed()};
                if (headerPath.endsWith(frameworkSuffix())) {
                    headerPath.truncate(headerPath.size() - frameworkSuffix().size());
                    prefix = QByteArrayLiteral("-F");
                }
                pathList.append(prefix + headerPath);
            }
        } else if (line.startsWith(QByteArrayLiteral("#include <...> search starts here"))) {
            isIncludeDir = true;
        }
    }

    return pathList;
}

std::vector<std::string> ClangCppParser::getAliasFunctionDefinition()
{
    QStringList aliases = trFunctionAliasManager.listAliases();
    std::vector<std::string> results;
    for (QString alias : aliases) {
        std::string definition = "-D" + alias.toStdString();
        switch (trFunctionAliasManager.trFunctionByName(alias)) {
        case TrFunctionAliasManager::Function_QT_TR_N_NOOP:
            definition += "(x)=QT_TR_N_NOOP(x)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_trUtf8:
        case TrFunctionAliasManager::Function_tr:
            definition += "=tr";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TR_NOOP:
            definition += "(x)=QT_TR_NOOP(x)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TR_NOOP_UTF8:
            definition += "(x)=QT_TR_NOOP_UTF8(x)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
            definition += "(scope,x)=QT_TRANSLATE_N_NOOP(scope,x)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
            definition += "(scope, x, comment)=QT_TRANSLATE_N_NOOP3(scope, x, comment)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_translate:
            definition += "=QCoreApplication::translate";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_findMessage:
            definition += "=findMessage";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
            definition += "(scope,x)=QT_TRANSLATE_NOOP(scope,x)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
            definition += "(scope,x)=QT_TRANSLATE_NOOP_UTF8(scope,x)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
            definition += "(scope, x, comment)=QT_TRANSLATE_NOOP3(scope, x, comment)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
            definition += "(scope, x, comment)=QT_TRANSLATE_NOOP3_UTF8(scope, x, comment)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_qtTrId:
            definition += "=qtTrId";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
        case TrFunctionAliasManager::Function_QT_TRID_NOOP:
            definition += "(id)=QT_TRID_NOOP(id)";
            results.push_back(definition);
            break;
        case TrFunctionAliasManager::Function_Q_DECLARE_TR_FUNCTIONS:
            definition += "(context)=Q_DECLARE_TR_FUNCTIONS(context)";
            results.push_back(definition);
            break;
        default:
            break;
        }
    }
    return results;
}

static std::vector<std::string> aliasDefinition;

static clang::tooling::ArgumentsAdjuster getClangArgumentAdjuster()
{
    const QByteArrayList compilerIncludeFlags = getIncludePathsFromCompiler();
    return [=](const clang::tooling::CommandLineArguments &args, llvm::StringRef /*unused*/) {
        clang::tooling::CommandLineArguments adjustedArgs(args);
        clang::tooling::CommandLineArguments adjustedArgsTemp;

        adjustedArgsTemp.push_back("-fparse-all-comments");
        adjustedArgsTemp.push_back("-nostdinc");

        // Turn off SSE support to avoid usage of gcc builtins.
        // TODO: Look into what Qt Creator does.
        // Pointers: HeaderPathFilter::removeGccInternalIncludePaths()
        //           and gccInstallDir() in gcctoolchain.cpp
        // Also needed for Mac, No need for CLANG_RESOURCE_DIR when this is part of the argument.
        adjustedArgsTemp.push_back("-mno-sse");

#ifdef Q_OS_WIN
        adjustedArgsTemp.push_back("-fms-compatibility-version=19");
        adjustedArgsTemp.push_back("-DQ_COMPILER_UNIFORM_INIT"); // qtbase + clang-cl hack
        // avoid constexpr error connected with offsetof (QTBUG-97380)
        adjustedArgsTemp.push_back("-D_CRT_USE_BUILTIN_OFFSETOF");
#endif
        adjustedArgsTemp.push_back("-Wno-everything");

        for (const QByteArray &flag : compilerIncludeFlags)
            adjustedArgsTemp.push_back(flag.data());

        for (auto alias : aliasDefinition) {
            adjustedArgsTemp.push_back(alias);
        }

        clang::tooling::CommandLineArguments::iterator it = llvm::find(adjustedArgs, "--");
        adjustedArgs.insert(it, adjustedArgsTemp.begin(), adjustedArgsTemp.end());
        return adjustedArgs;
    };
}

bool ClangCppParser::stringContainsTranslationInformation(llvm::StringRef ba)
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

    const size_t pos = ba.find_first_of("QT_TR");
    const auto baSliced = ba.slice(pos, llvm::StringRef::npos);
     if (pos != llvm::StringRef::npos) {
         if (baSliced.contains(qtTrNoop) || baSliced.contains(qtTrNoopUTF8) || baSliced.contains(qtTrNNoop)
             || baSliced.contains(qtTrIdNoop) || baSliced.contains(qtTrIdNNoop)
             || baSliced.contains(qtTranslateNoop) ||  baSliced.contains(qtTranslateNoopUTF8)
             ||  baSliced.contains(qtTranslateNNoop) || baSliced.contains(qtTranslateNoop3)
             ||  baSliced.contains(qtTranslateNoop3UTF8) ||  baSliced.contains(qtTranslateNNoop3))
             return true;
     }

     if (ba.contains(qDeclareTrFunction) || ba.contains(translatorComment) || ba.contains(qtTrId) || ba.contains(tr)
         || ba.contains(trUtf8) || ba.contains(translate))
        return true;

     for (QString alias : trFunctionAliasManager.listAliases()) {
         if (ba.contains(qPrintable(alias)))
             return true;
     }


     return false;
}

static bool generateCompilationDatabase(const QString &outputFilePath, const ConversionData &cd)
{
    QJsonArray commandObjects;
    const QString buildDir = QDir::currentPath();
    const QString fakefileName = QLatin1String("dummmy.cpp");
    QJsonObject obj;
    obj[QLatin1String("file")] = fakefileName;
    obj[QLatin1String("directory")] = buildDir;
    QJsonArray args = {
            QLatin1String("clang++"),
            QLatin1String("-std=gnu++17"),
    #ifndef Q_OS_WIN
            QLatin1String("-fPIC"),
    #endif
        };

#if defined(Q_OS_MACOS) && QT_CONFIG(framework)
        const QString installPath = QLibraryInfo::path(QLibraryInfo::LibrariesPath);
        QString arg =  QLatin1String("-F") + installPath;
        args.push_back(arg);
#endif

    for (const QString &path : cd.m_includePath) {
            QString arg = QLatin1String("-I") + path;
            args.push_back(std::move(arg));
    }

    obj[QLatin1String("arguments")] = args;
    commandObjects.append(obj);

    QJsonDocument doc(commandObjects);
    QFile file(outputFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(doc.toJson());
    return true;
}

// Sort messages in such a way that they appear in the same order like in the given file list.
static void sortMessagesByFileOrder(ClangCppParser::TranslatorMessageVector &messages,
                                    const QStringList &files)
{
    // first sort messages by line number
    std::stable_sort(messages.begin(), messages.end(),
              [&](const TranslatorMessage &lhs, const TranslatorMessage &rhs) {
                  auto i = lhs.lineNumber();
                  auto k = rhs.lineNumber();
                  return i < k;
              });

    QHash<QString, QStringList::size_type> indexByPath;
    for (const TranslatorMessage &m : messages)
        indexByPath[m.fileName()] = std::numeric_limits<QStringList::size_type>::max();

    for (QStringList::size_type i = 0; i < files.size(); ++i)
        indexByPath[files[i]] = i;

    std::stable_sort(messages.begin(), messages.end(),
              [&](const TranslatorMessage &lhs, const TranslatorMessage &rhs) {
                  auto i = indexByPath.value(lhs.fileName());
                  auto k = indexByPath.value(rhs.fileName());
                  return i < k;
              });
}

bool ClangCppParser::hasAliases()
{
    QStringList listAlias = trFunctionAliasManager.listAliases();
    if (listAlias.size() > 0)
        return true;
    return false;
}

void ClangCppParser::loadCPP(Translator &translator, const QStringList &files, ConversionData &cd,
                            bool *fail)
{
    FileSignificanceCheck::create();
    auto cleanup = qScopeGuard(FileSignificanceCheck::destroy);
    FileSignificanceCheck::the()->setExclusionPatterns(cd.m_excludes);
    if (cd.m_rootDirs.size() > 0)
        FileSignificanceCheck::the()->setRootDirectories(cd.m_rootDirs);
    else
        FileSignificanceCheck::the()->setRootDirectories(getProjectDirsFromEnvironment());

    if (hasAliases())
        aliasDefinition = getAliasFunctionDefinition();

    // pre-process the files by a simple text search if there is any occurrence
    // of things we are interested in
    qCDebug(lcClang) << "Load CPP \n";
    std::vector<std::string> sources;
    for (const QString &filename : files) {
        qCDebug(lcClang) << "File: " << filename << " \n";
        sources.emplace_back(filename.toStdString());
    }

    std::string errorMessage;
    std::unique_ptr<CompilationDatabase> db;
    if (cd.m_compilationDatabaseDir.isEmpty()) {
        db = CompilationDatabase::autoDetectFromDirectory(".", errorMessage);
        if (!db && !files.isEmpty()) {
            db = CompilationDatabase::autoDetectFromSource(files.first().toStdString(),
                                                           errorMessage);
        }
    } else {
        db = CompilationDatabase::autoDetectFromDirectory(cd.m_compilationDatabaseDir.toStdString(),
                                                          errorMessage);
    }

    if (!db) {
        const QString dbFilePath = QStringLiteral("compile_commands.json");
        qCDebug(lcClang) << "Generating compilation database" << dbFilePath;
        if (!generateCompilationDatabase(dbFilePath, cd)) {
            *fail = true;
            cd.appendError(u"Cannot generate compilation database."_s);
            return;
        }
        errorMessage.clear();
        db = CompilationDatabase::loadFromDirectory(".", errorMessage);
    }

    if (!db) {
        *fail = true;
        cd.appendError(QString::fromStdString(errorMessage));
        return;
    }

    TranslationStores ast, qdecl, qnoop;
    Stores stores(ast, qdecl, qnoop);

    std::vector<std::thread> producers;
    ReadSynchronizedRef<std::string> ppSources(sources);
    WriteSynchronizedRef<TranslationRelatedStore> ppStore(stores.Preprocessor);
    size_t idealProducerCount = std::min(ppSources.size(), size_t(std::thread::hardware_concurrency()));
    clang::tooling::ArgumentsAdjuster argumentsAdjusterSyntaxOnly =
            clang::tooling::getClangSyntaxOnlyAdjuster();
    clang::tooling::ArgumentsAdjuster argumentsAdjusterLocal = getClangArgumentAdjuster();
    clang::tooling::ArgumentsAdjuster argumentsAdjuster =
            clang::tooling::combineAdjusters(argumentsAdjusterLocal, argumentsAdjusterSyntaxOnly);

    for (size_t i = 0; i < idealProducerCount; ++i) {
        std::thread producer([&ppSources, &db, &ppStore, &argumentsAdjuster]() {
            std::string file;
            while (ppSources.next(&file)) {
                clang::tooling::ClangTool tool(*db, file);
                tool.appendArgumentsAdjuster(argumentsAdjuster);
                tool.run(new LupdatePreprocessorActionFactory(&ppStore));
            }
        });
        producers.emplace_back(std::move(producer));
    }
    for (auto &producer : producers)
        producer.join();
    producers.clear();

    ReadSynchronizedRef<std::string> astSources(sources);
    idealProducerCount = std::min(astSources.size(), size_t(std::thread::hardware_concurrency()));
    for (size_t i = 0; i < idealProducerCount; ++i) {
        std::thread producer([&astSources, &db, &stores, &argumentsAdjuster]() {
            std::string file;
            while (astSources.next(&file)) {
                clang::tooling::ClangTool tool(*db, file);
                tool.appendArgumentsAdjuster(argumentsAdjuster);
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

    ReadSynchronizedRef<TranslationRelatedStore> rsvQNoop(qnoop);
    //unlike ast translation context, qnoop context don't need to be corrected
    //(because Q_DECLARE_TR_FUNCTION context is already applied).
    ClangCppParser::finalize(rsvQNoop, wsv);

    TranslatorMessageVector messages;
    for (auto &store : finalStores)
        ClangCppParser::collectMessages(messages, store);

    sortMessagesByFileOrder(messages, files);

    for (TranslatorMessage &msg : messages) {
        if (!msg.warning().isEmpty()) {
            std::cerr << qPrintable(msg.warning());
            if (msg.warningOnly() == true)
                continue;
        }
        translator.extend(std::move(msg), cd);
    }
}

void ClangCppParser::collectMessages(TranslatorMessageVector &result,
                                     TranslationRelatedStore &store)
{
    if (!store.isValid(true)) {
        if (store.lupdateWarning.isEmpty())
            return;
        // The message needs to be added to the results so that the warning can be ordered
        // and printed in a consistent way.
        // the message won't appear in the .ts file
        result.push_back(translatorMessage(store, store.lupdateIdMetaData, false, false, true));
        return;
    }

    qCDebug(lcClang) << "---------------------------------------------------------------Filling translator for " << store.funcName;
    qCDebug(lcClang) << " contextRetrieved " << store.contextRetrieved;
    qCDebug(lcClang) << " source   " << store.lupdateSource;

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
        if (!store.lupdateSourceWhenId.isEmpty()) {
            std::stringstream warning;
            warning << qPrintable(store.lupdateLocationFile) << ":"
                << store.lupdateLocationLine << ":"
                << store.locationCol << ": "
                << "//% cannot be used with tr() / QT_TR_NOOP(). Ignoring\n";
            store.lupdateWarning.append(QString::fromStdString(warning.str()));
            qCDebug(lcClang) << "//% is ignored when using tr function\n";
        }
        if (store.contextRetrieved.isEmpty() && store.contextArg.isEmpty()) {
            std::stringstream warning;
            warning << qPrintable(store.lupdateLocationFile) << ":"
                << store.lupdateLocationLine << ":"
                << store.locationCol << ": "
                << qPrintable(store.funcName) << " cannot be called without context."
                << " The call is ignored (missing Q_OBJECT maybe?)\n";
            store.lupdateWarning.append(QString::fromStdString(warning.str()));
            qCDebug(lcClang) << "tr() cannot be called without context \n";
            // The message need to be added to the results so that the warning can be ordered
            // and printed in a consistent way.
            // the message won't appear in the .ts file
            result.push_back(translatorMessage(store, store.lupdateIdMetaData, plural, false, true));
        } else
            result.push_back(translatorMessage(store, store.lupdateIdMetaData, plural, false));
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
        if (!store.lupdateSourceWhenId.isEmpty()) {
            std::stringstream warning;
            warning << qPrintable(store.lupdateLocationFile) << ":"
                << store.lupdateLocationLine << ":"
                << store.locationCol << ": "
                << "//% cannot be used with translate() / QT_TRANSLATE_NOOP(). Ignoring\n";
            store.lupdateWarning.append(QString::fromStdString(warning.str()));
            qCDebug(lcClang) << "//% is ignored when using translate function\n";
        }
        result.push_back(translatorMessage(store, store.lupdateIdMetaData, plural, false));
        break;

    // handle qtTrId
    case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
        plural = true;
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_qtTrId:
    case TrFunctionAliasManager::Function_QT_TRID_NOOP:
        if (!store.lupdateIdMetaData.isEmpty()) {
            std::stringstream warning;
            warning << qPrintable(store.lupdateLocationFile) << ":"
                << store.lupdateLocationLine << ":"
                << store.locationCol << ": "
                << "//= cannot be used with qtTrId() / QT_TRID_NOOP(). Ignoring\n";
            store.lupdateWarning.append(QString::fromStdString(warning.str()));
            qCDebug(lcClang) << "//= is ignored when using qtTrId function \n";
        }
        result.push_back(translatorMessage(store, store.lupdateId, plural, true));
        break;
    default:
        if (store.funcName == QStringLiteral("TRANSLATOR"))
            result.push_back(translatorMessage(store, store.lupdateIdMetaData, plural, false));
    }
}

static QString ensureCanonicalPath(const QString &filePath)
{
    QFileInfo fi(filePath);
    if (fi.isRelative())
        fi.setFile(QDir::current().absoluteFilePath(filePath));
    return fi.canonicalFilePath();
}

TranslatorMessage ClangCppParser::translatorMessage(const TranslationRelatedStore &store,
    const QString &id, bool plural, bool isId, bool isWarningOnly)
{
    if (isWarningOnly) {
        TranslatorMessage msg;
        // msg filled with file name and line number should be enough for the message ordering
        msg.setFileName(ensureCanonicalPath(store.lupdateLocationFile));
        msg.setLineNumber(store.lupdateLocationLine);
        msg.setWarning(store.lupdateWarning);
        msg.setWarningOnly(isWarningOnly);
        return msg;
    }

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
        ensureCanonicalPath(store.lupdateLocationFile),
        store.lupdateLocationLine,
        QStringList(),
        TranslatorMessage::Type::Unfinished,
        (plural ? plural : !store.lupdatePlural.isEmpty()));

    if (!store.lupdateAllMagicMetaData.empty())
        msg.setExtras(store.lupdateAllMagicMetaData);
    msg.setExtraComment(ParserTool::transcode(store.lupdateExtraComment));
    msg.setId(ParserTool::transcode(id));
    if (!store.lupdateWarning.isEmpty())
        msg.setWarning(store.lupdateWarning);
    return msg;
}

#define START_THREADS(RSV, WSV) \
    std::vector<std::thread> producers; \
    const size_t idealProducerCount = std::min(RSV.size(), size_t(std::thread::hardware_concurrency())); \
    \
    for (size_t i = 0; i < idealProducerCount; ++i) { \
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

void ClangCppParser::finalize(ReadSynchronizedRef<TranslationRelatedStore> &ast,
    WriteSynchronizedRef<TranslationRelatedStore> &newAst)
{
    START_THREADS(ast, newAst)
    JOIN_THREADS(newAst)
}

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
        if (store.contextRetrieved == declareStore.contextRetrieved) {
            qCDebug(lcClang) << "* Tr call context retrieved " << store.contextRetrieved;
            qCDebug(lcClang) << "* Tr call source            " << store.lupdateSource;
            qCDebug(lcClang) << "* DECLARE context retrieved " << declareStore.contextRetrieved;
            qCDebug(lcClang) << "* DECLARE context Arg       " << declareStore.contextArg;
            store.contextRetrieved = declareStore.contextArg;
            // store.contextArg should never be overwritten.
            break;
        }
    }

    JOIN_THREADS(newAst)
}

#undef START_THREADS
#undef JOIN_THREADS

QT_END_NAMESPACE
