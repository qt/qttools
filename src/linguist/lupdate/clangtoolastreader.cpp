// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "clangtoolastreader.h"
#include "filesignificancecheck.h"
#include "translator.h"

#include <QLibraryInfo>

QT_BEGIN_NAMESPACE

namespace LupdatePrivate
{
    void exploreChildrenForFirstStringLiteral(clang::Stmt* stmt, QString &context)
    {
        // only exploring the children until the context has been found.
        if (!stmt || !context.isEmpty())
            return;

        for (auto it = stmt->child_begin() ; it !=stmt->child_end() ; it++) {
            if (!context.isEmpty())
                break;
            clang::Stmt *child = *it;
            clang::StringLiteral *stringLit = llvm::dyn_cast_or_null<clang::StringLiteral>(child);
            if (stringLit) {
                context = toQt(stringLit->getString());
                return;
            }
            exploreChildrenForFirstStringLiteral(child, context);
        }
        return;
    }

    // Checks if the tr method is supported by the CXXRecordDecl
    // Either because Q_OBJECT or Q_DECLARE_FUNCTIONS(MyContext) is declared with this CXXRecordDecl
    // In case of Q_DECLARE_FUNCTIONS the context is read in the tr Method children with function exploreChildrenForFirstStringLiteral
    // Q_DECLARE_FUNCTIONS trace in the AST is:
    // - a public AccessSpecDecl pointing to src/corelib/kernel/qcoreapplication.h
    // - a CXXMethodDecl called tr with a children that is a StringLiteral. This is the context
    // Q_OBJECT trace in the AST is:
    // - a public AccessSpecDecl pointing to src/corelib/kernel/qtmetamacros.h
    // - a CXXMethodDecl called tr WITHOUT a StringLiteral among its children.
    bool isQObjectOrQDeclareTrFunctionMacroDeclared(clang::CXXRecordDecl *recordDecl, QString &context, const clang::SourceManager &sm)
    {
        if (!recordDecl)
            return false;

        bool tr_method_present = false;
        bool access_for_qobject = false;
        bool access_for_qdeclaretrfunction = false;

        for (auto decl : recordDecl->decls()) {
            clang::AccessSpecDecl *accessSpec = llvm::dyn_cast<clang::AccessSpecDecl>(decl);
            clang::CXXMethodDecl *method = llvm::dyn_cast<clang::CXXMethodDecl>(decl);

            if (!accessSpec && !method)
                continue;
            if (method) {
                // Look for method with name 'tr'
                std::string name = method->getNameAsString();
                if (name == "tr") {
                    tr_method_present = true;
                    // if nothing is found and the context remains empty, it's ok, it's probably a Q_OBJECT.
                    exploreChildrenForFirstStringLiteral(method->getBody(), context);
                }
            } else if (accessSpec) {
                if (!accessSpec->getBeginLoc().isValid())
                        continue;
                QString location = QString::fromStdString(
                            sm.getSpellingLoc(accessSpec->getBeginLoc()).printToString(sm));
                qsizetype indexLast = location.lastIndexOf(QLatin1String(":"));
                qsizetype indexBeforeLast = location.lastIndexOf(QLatin1String(":"), indexLast-1);
                location.truncate(indexBeforeLast);
                const QString qtInstallDirPath = QLibraryInfo::path(QLibraryInfo::PrefixPath);
                const QString accessForQDeclareTrFunctions = QStringLiteral("qcoreapplication.h");
                const QString accessForQObject = QStringLiteral("qtmetamacros.h");
                // Qt::CaseInsensitive because of potential discrepancy in Windows with D:/ and d:/
                if (location.startsWith(qtInstallDirPath, Qt::CaseInsensitive)) {
                    if (location.endsWith(accessForQDeclareTrFunctions))
                        access_for_qdeclaretrfunction = true;
                    if (location.endsWith(accessForQObject))
                        access_for_qobject = true;
                }
            }
        }

        bool access_to_qtbase = false;
        // if the context is still empty then it cannot be a Q_DECLARE_TR_FUNCTION.
        if (context.isEmpty())
            access_to_qtbase = access_for_qobject;
        else
            access_to_qtbase = access_for_qdeclaretrfunction;

        return tr_method_present && access_to_qtbase;
    }

    QString exploreBases(clang::CXXRecordDecl *recordDecl, const clang::SourceManager &sm);
    QString lookForContext(clang::CXXRecordDecl *recordDecl, const clang::SourceManager &sm)
    {
        QString context;
        if (isQObjectOrQDeclareTrFunctionMacroDeclared(recordDecl, context, sm)) {
            return context.isEmpty() ? QString::fromStdString(recordDecl->getQualifiedNameAsString()) : context;
        } else {
            // explore the bases of this CXXRecordDecl
            // the base class AA takes precedent over B (reproducing tr context behavior)
            /*
             class AA {Q_OBJECT};
             class A : public AA {};
             class B {
                 Q_OBJECT
                 class C : public A
                 {
                     QString c_tr = tr("context is AA");
                     const char * c_noop = QT_TR_NOOP("context should be AA");
                 }
             };
            */
            // For recordDecl corresponding to class C, the following gives access to class A
            return exploreBases(recordDecl, sm);
        }
    }

    // Gives access to the class or struct the CXXRecordDecl is inheriting from
    QString exploreBases(clang::CXXRecordDecl *recordDecl, const clang::SourceManager &sm)
    {
        QString context;
        for (auto base : recordDecl->bases()) {
            const clang::Type *type = base.getType().getTypePtrOrNull();
            if (!type) continue;
            clang::CXXRecordDecl *baseDecl = type->getAsCXXRecordDecl();
            if (!baseDecl)
                continue;
            context = lookForContext(baseDecl, sm);
            if (!context.isEmpty())
                return context;
        }
        return context;
    }

    // QT_TR_NOOP location is within the the NamedDecl range
    // Look for the RecordDecl (class or struct) the NamedDecl belongs to
    // and the related classes until Q_OBJECT macro declaration or Q_DECLARE_TR_FUNCTIONS is found.
    // The first class where Q_OBJECT or Q_DECLARE_TR_FUNCTIONS is declared is the context.
    // The goal is to reproduce the behavior exibited by the new parser for tr function.
    // tr function and QT_TR_NOOP, when next to each other in code, should always have the same context!
    //
    // The old parser does not do this.
    // If a Q_OBJECT macro cannot be found in the first class
    // a warning is emitted and the class is used as context regardless.
    // This is the behavior for tr function and QT_TR_NOOP
    // This is not correct.
    QString contextForNoopMacro(clang::NamedDecl *namedDecl, const clang::SourceManager &sm)
    {
        QString context;
        clang::DeclContext *decl = namedDecl->getDeclContext();
        if (!decl)
            return context;
        while (decl) {
            qCDebug(lcClang) << "--------------------- decl kind name: " << decl->getDeclKindName();
            if (clang::isa<clang::CXXRecordDecl>(decl)) {
                clang::CXXRecordDecl *recordDecl = llvm::dyn_cast<clang::CXXRecordDecl>(decl);

                context = lookForContext(recordDecl, sm);

                if (!context.isEmpty())
                    return context;
            }
            decl = decl->getParent(); // Brings to the class or struct decl is nested in, if it exists.
        }

        // If no context has been found: do not emit a warning here.
        // because more than one NamedDecl can include the QT_TR_NOOP macro location
        // in the following, class A and class B and c_noop will.
        /*
        class A {
            class B
            {
                Q_OBJECT
                const char * c_noop = QT_TR_NOOP("context is B");
            }
        };
        */
        // calling contextForNoopMacro on NamedDecl corresponding to class A
        // no context will be found, but it's ok because the context will be found
        // when the function is called on c_noop.
        return context;
    }


    QString contextForFunctionDecl(clang::FunctionDecl *func, const std::string &funcName)
    {
        std::string context;
#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
        {
            llvm::raw_string_ostream tmp(context);
            func->printQualifiedName(tmp);
        }
#else
        context = func->getQualifiedNameAsString();
#endif
        return QString::fromStdString(context.substr(0, context.find("::" + funcName, 0)));
    }

    static bool capture(const QRegularExpression &exp, const QString &line, QString *i, QString *c)
    {
        i->clear(), c->clear();
        auto result = exp.match(line);
        if (!result.hasMatch())
            return false;

        *i = result.captured(QLatin1String("identifier"));
        *c = result.captured(QStringLiteral("comment")).trimmed();

        if (*i == QLatin1String("%"))
            *c = LupdatePrivate::cleanQuote(c->toStdString(), QuoteCompulsary::Left);

        return !c->isEmpty();
    }

    bool hasQuote(llvm::StringRef source)
    {
        return source.contains("\"");
    }

    bool trFunctionPresent(llvm::StringRef text)
    {
        if (text.contains(llvm::StringRef("qtTrId(")))
            return true;
        if (text.contains(llvm::StringRef("tr(")))
            return true;
        if (text.contains(llvm::StringRef("trUtf8(")))
            return true;
        if (text.contains(llvm::StringRef("translate(")))
            return true;
        if (text.contains(llvm::StringRef("Q_DECLARE_TR_FUNCTIONS(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TR_N_NOOP(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRID_N_NOOP(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRANSLATE_N_NOOP(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRANSLATE_N_NOOP3(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TR_NOOP(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRID_NOOP(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRANSLATE_NOOP(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRANSLATE_NOOP3(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TR_NOOP_UTF8(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRANSLATE_NOOP_UTF8(")))
            return true;
        if (text.contains(llvm::StringRef("QT_TRANSLATE_NOOP3_UTF8(")))
            return true;
        return false;
    }

    bool isPointWithin(const clang::SourceRange &sourceRange, const clang::SourceLocation &point,
        const clang::SourceManager &sm)
    {
        clang::SourceLocation start = sourceRange.getBegin();
        clang::SourceLocation end = sourceRange.getEnd();
        return point == start || point == end || (sm.isBeforeInTranslationUnit(start, point)
            && sm.isBeforeInTranslationUnit(point, end));
    }

    class BeforeThanCompare
    {
        const clang::SourceManager &SM;

    public:
        explicit BeforeThanCompare(const clang::SourceManager &SM) : SM(SM) { }

        bool operator()(const clang::RawComment &LHS, const clang::RawComment &RHS)
        {
            return SM.isBeforeInTranslationUnit(LHS.getBeginLoc(), RHS.getBeginLoc());
        }

        bool operator()(const clang::RawComment *LHS, const clang::RawComment *RHS)
        {
            return operator()(*LHS, *RHS);
        }
    };
}

/*
    The visit call expression function is called automatically after the
    visitor TraverseAST function is called. This is the function where the
    "tr", "trUtf8", "qtIdTr", "translate" functions are picked up in the AST.
    Previously mentioned functions are always part of a CallExpression.
*/
bool LupdateVisitor::VisitCallExpr(clang::CallExpr *callExpression)
{
    const auto fullLocation = m_context->getFullLoc(callExpression->getBeginLoc());
    if (fullLocation.isInvalid())
        return true;
    clang::FunctionDecl *func = callExpression->getDirectCallee();
    if (!func)
        return true;
    clang::QualType q = callExpression->getType();
    if (!q.getTypePtrOrNull())
        return true;

    struct {
        unsigned Line;
        std::string Filename;
    } info;

    const auto funcName = QString::fromStdString(func->getNameInfo().getAsString());

    // Only continue if the function a translation function (TODO: deal with alias function...)
    switch (trFunctionAliasManager.trFunctionByName(funcName)) {
    case TrFunctionAliasManager::Function_tr:
    case TrFunctionAliasManager::Function_trUtf8:
    case TrFunctionAliasManager::Function_translate:
    case TrFunctionAliasManager::Function_qtTrId:{

        const auto &sm = m_context->getSourceManager();
        const auto fileLoc = sm.getFileLoc(callExpression->getBeginLoc());
        if (fileLoc.isInvalid() || !fileLoc.isFileID())
            return true;
        // not using line directive (# line)
        auto presumedLoc = sm.getPresumedLoc(fileLoc, false);
        if (presumedLoc.isInvalid())
            return true;
        info = { presumedLoc.getLine(), presumedLoc.getFilename() };
    }   break;
    default:
        return true;
    }

    // Checking that the CallExpression is from the input file we're interested in
    if (!LupdatePrivate::isFileSignificant(info.Filename))
        return true;

    qCDebug(lcClang) << "************************** VisitCallExpr ****************";
    // Retrieving the information needed to fill the lupdate translator.
    // Function independent retrieve
    TranslationRelatedStore store;
    store.callType = QStringLiteral("ASTRead_CallExpr");
    store.funcName = funcName;
    store.lupdateLocationFile = QString::fromStdString(info.Filename);
    store.lupdateLocationLine = info.Line;
    store.contextRetrieved = LupdatePrivate::contextForFunctionDecl(func, funcName.toStdString());

    qCDebug(lcClang) << "CallType          : ASTRead_CallExpr";
    qCDebug(lcClang) << "Function name     : " << store.funcName;
    qCDebug(lcClang) << "File location     : " << store.lupdateLocationFile;
    qCDebug(lcClang) << "Line              : " << store.lupdateLocationLine;
    qCDebug(lcClang) << "Context retrieved : " << store.contextRetrieved;

    // Here we gonna need to retrieve the comments around the function call
    // //: //* //~ Things like that
    const std::vector<QString> rawComments = rawCommentsForCallExpr(callExpression);
    for (const auto &rawComment : rawComments) {
        setInfoFromRawComment(rawComment, &store);
        qCDebug(lcClang) << "Raw comments     :" << rawComment;
    }

    clang::LangOptions langOpts;
    langOpts.CPlusPlus = true;
    clang::PrintingPolicy policy(langOpts);
    std::vector<std::string> arguments(callExpression->getNumArgs(), "");
    for (unsigned int i = 0; i < callExpression->getNumArgs(); i++) {
        auto arg = callExpression->getArg(i);
        llvm::raw_string_ostream temp(arguments[i]);
        arg->printPretty(temp, nullptr, policy);
    }

    // Function dependent retrieve!
    switch (trFunctionAliasManager.trFunctionByName(funcName)) {
    case TrFunctionAliasManager::Function_tr:
    case TrFunctionAliasManager::Function_trUtf8:
        if (arguments.size() != 3 || !LupdatePrivate::hasQuote(arguments[0]))
            return true;
        store.lupdateSource = LupdatePrivate::cleanQuote(arguments[0]);
        store.lupdateComment = LupdatePrivate::cleanQuote(arguments[1]);
        store.lupdatePlural = QString::fromStdString(arguments[2]);
        qCDebug(lcClang) << "Source      : " << store.lupdateSource;
        qCDebug(lcClang) << "Comment     : " << store.lupdateComment;
        qCDebug(lcClang) << "Plural      : " << store.lupdatePlural;
        break;
    case TrFunctionAliasManager::Function_translate:
        if (arguments.size() != 4 || !LupdatePrivate::hasQuote(arguments[0])
            || !LupdatePrivate::hasQuote(arguments[1])) {
            return true;
        }
        store.contextArg = LupdatePrivate::cleanQuote(arguments[0]);
        store.lupdateSource = LupdatePrivate::cleanQuote(arguments[1]);
        store.lupdateComment = LupdatePrivate::cleanQuote(arguments[2]);
        store.lupdatePlural = QString::fromStdString(arguments[3]);
        qCDebug(lcClang) << "Context Arg : " << store.contextArg;
        qCDebug(lcClang) << "Source      : " << store.lupdateSource;
        qCDebug(lcClang) << "Comment     : " << store.lupdateComment;
        qCDebug(lcClang) << "Plural      : " << store.lupdatePlural;
        break;
    case TrFunctionAliasManager::Function_qtTrId:
        if (arguments.size() != 2 || !LupdatePrivate::hasQuote(arguments[0]))
            return true;
        store.lupdateId = LupdatePrivate::cleanQuote(arguments[0]);
        store.lupdatePlural = QString::fromStdString(arguments[1]);
        qCDebug(lcClang) << "ID          : " << store.lupdateId;
        qCDebug(lcClang) << "Plural      : " << store.lupdatePlural;
        break;
    }
    // locationCol needs to be set for the store to be considered valid (but really only needed for PP calls, to reconstruct location)
    store.locationCol = 0;
    m_trCalls.emplace_back(std::move(store));
    return true;
}

void LupdateVisitor::processIsolatedComments()
{
    auto &sourceMgr = m_context->getSourceManager();
    processIsolatedComments(sourceMgr.getMainFileID()) ;
}

/*
    Retrieve the comments not associated with tr calls.
*/
void LupdateVisitor::processIsolatedComments(const clang::FileID file)
{
    qCDebug(lcClang) << "==== processIsolatedComments ====";
    auto &sourceMgr = m_context->getSourceManager();

#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
    const auto commentsInThisFile = m_context->Comments.getCommentsInFile(file);
    if (!commentsInThisFile)
        return;

    std::vector<clang::RawComment *> tmp;
    for (const auto &commentInFile : *commentsInThisFile)
        tmp.emplace_back(commentInFile.second);
    clang::ArrayRef<clang::RawComment *> rawComments = tmp;
#else
    Q_UNUSED(file);
    clang::ArrayRef<clang::RawComment *> rawComments = m_context->getRawCommentList().getComments();
#endif

    // If there are no comments anywhere, we won't find anything.
    if (rawComments.empty())
        return;

    // Searching for the comments of the form:
    // /*  TRANSLATOR CONTEXT
    //     whatever */
    // They are not associated to any tr calls
    // Each one needs its own entry in the m_stores->AST translation store
    for (const auto &rawComment : rawComments) {
        if (!LupdatePrivate::isFileSignificant(sourceMgr.getFilename(rawComment->getBeginLoc()).str()))
            continue;
        // Comments not separated by an empty line will be part of the same Raw comments
        // Each one needs to be saved with its line number.
        // The store is used here only to pass this information.
        TranslationRelatedStore store;
        store.lupdateLocationLine = sourceMgr.getPresumedLoc(rawComment->getBeginLoc(), false).getLine();
        store.lupdateLocationFile = QString::fromStdString(
                    sourceMgr.getPresumedLoc(rawComment->getBeginLoc(), false).getFilename());
        QString comment = toQt(rawComment->getRawText(sourceMgr));
        qCDebug(lcClang) << " raw Comment : \n" << comment;
        setInfoFromRawComment(comment, &store);
    }
}

/*
    Retrieve the comments associated with the CallExpression.
*/
std::vector<QString> LupdateVisitor::rawCommentsForCallExpr(const clang::CallExpr *callExpr) const
{
    if (!m_context)
        return {};
    return rawCommentsFromSourceLocation(m_context->getFullLoc(callExpr->getBeginLoc()));
}

std::vector<QString> LupdateVisitor::rawCommentsFromSourceLocation(
    clang::SourceLocation sourceLocation) const
{
    if (!m_context)
        return {};
    if (sourceLocation.isInvalid() || !sourceLocation.isFileID()) {
        qCDebug(lcClang) << "The declaration does not map directly to a location in a file,"
            " early return.";
        return {};
    }
    auto &sourceMgr = m_context->getSourceManager();

#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
    const clang::FileID file = sourceMgr.getDecomposedLoc(sourceLocation).first;
    const auto commentsInThisFile = m_context->Comments.getCommentsInFile(file);
    if (!commentsInThisFile)
        return {};

    std::vector<clang::RawComment *> tmp;
    for (const auto &commentInFile : *commentsInThisFile)
        tmp.emplace_back(commentInFile.second);
    clang::ArrayRef<clang::RawComment *> rawComments = tmp;
#else
    clang::ArrayRef<clang::RawComment *> rawComments = m_context->getRawCommentList().getComments();
#endif

    // If there are no comments anywhere, we won't find anything.
    if (rawComments.empty())
        return {};

    // Create a dummy raw comment with the source location of the declaration.
    clang::RawComment commentAtDeclarationLocation(sourceMgr,
        clang::SourceRange(sourceLocation), m_context->getLangOpts().CommentOpts, false);

    // Create a functor object to compare the source location of the comment and the declaration.
    const LupdatePrivate::BeforeThanCompare compareSourceLocation(sourceMgr);
    //  Find the comment that occurs just after or within this declaration. Possible findings:
    //  QObject::tr(/* comment 1 */ "test"); //: comment 2   -> finds "//: comment 1"
    //  QObject::tr("test"); //: comment 1                   -> finds "//: comment 1"
    //  QObject::tr("test");
    //  //: comment 1                                        -> finds "//: comment 1"
    //  /*: comment 1 */ QObject::tr("test");                -> finds no trailing comment
    auto comment = std::lower_bound(rawComments.begin(), rawComments.end(),
        &commentAtDeclarationLocation, compareSourceLocation);

    // We did not find any comment before the declaration.
    if (comment == rawComments.begin())
        return {};

    // Decompose the location for the declaration and find the beginning of the file buffer.
    std::pair<clang::FileID, unsigned> declLocDecomp = sourceMgr.getDecomposedLoc(sourceLocation);

    // Get the text buffer from the beginning of the file up through the declaration's begin.
    bool invalid = false;
    const char *buffer = sourceMgr.getBufferData(declLocDecomp.first, &invalid).data();
    if (invalid) {
        qCDebug(lcClang).nospace() << "An error occurred fetching the source buffer of file: "
                                   << toQt(sourceMgr.getFilename(sourceLocation));
        return {};
    }

    std::vector<QString> retrievedRawComments;
    auto lastDecompLoc = declLocDecomp.second;
    const auto declLineNum = sourceMgr.getLineNumber(declLocDecomp.first, declLocDecomp.second);
    do {
        std::advance(comment, -1);

        // Decompose the end of the comment.
        std::pair<clang::FileID, unsigned> commentEndDecomp
            = sourceMgr.getDecomposedLoc((*comment)->getSourceRange().getEnd());

        // If the comment and the declaration aren't in the same file, then they aren't related.
        if (declLocDecomp.first != commentEndDecomp.first) {
            qCDebug(lcClang) << "Comment and the declaration aren't in the same file. Comment '"
                             << toQt((*comment)->getRawText(sourceMgr)) << "' is ignored, return.";
            return retrievedRawComments;
        }

        // Current lupdate ignores comments on the same line before the declaration.
        // void Class42::hello(int something /*= 17 */, QString str = Class42::tr("eyo"))
        bool sameLineComment = false;
        if (declLineNum == sourceMgr.getLineNumber(commentEndDecomp.first, commentEndDecomp.second))
            sameLineComment = true;

        // Extract text between the comment and declaration.
        llvm::StringRef text(buffer + commentEndDecomp.second,
            lastDecompLoc - commentEndDecomp.second);

        // There should be no other declarations or preprocessor directives between
        // comment and declaration.
        if (text.find_first_of(";}#@") != llvm::StringRef::npos) {
            qCDebug(lcClang) << "Found another declaration or preprocessor directive between"
                " comment and declaration, break.";
            break;
        }
        if (sameLineComment && text.find_first_of(",") != llvm::StringRef::npos) {
            qCDebug(lcClang) << "Comment ends on same line as the declaration and is separated "
                                "from the tr call by a ','. Comment '"
                             << toQt((*comment)->getRawText(sourceMgr))
                             << "' is ignored, continue.";
            continue; // if there is a comment on the previous line it should be picked up
        }

        // There should be no other translation function between comment and declaration.
        if (LupdatePrivate::trFunctionPresent(text)) {
            qCDebug(lcClang) << "Found another translation function between comment and "
                "declaration, break.";
            break;
        }

        retrievedRawComments.emplace(retrievedRawComments.begin(),
                                     toQt((*comment)->getRawText(sourceMgr)));
        lastDecompLoc = sourceMgr.getDecomposedLoc((*comment)->getSourceRange().getBegin()).second;
    } while (comment != rawComments.begin());

    return retrievedRawComments;
}

/*
    Read the raw comments and split them according to the prefix.
    Fill the corresponding variables in the TranslationRelatedStore.
*/
void LupdateVisitor::setInfoFromRawComment(const QString &commentString,
    TranslationRelatedStore *store)
{
    const QStringList commentLines = commentString.split(QLatin1Char('\n'));

    static const QRegularExpression
        cppStyle(
        QStringLiteral("^\\/\\/(?<identifier>[:=~%]|(\\s*?TRANSLATOR))\\s+(?<comment>.+)$"));
    static const QRegularExpression
        cStyleSingle(
        QStringLiteral("^\\/\\*(?<identifier>[:=~%]|(\\s*?TRANSLATOR))\\s+(?<comment>.+)\\*\\/$"));
    static const QRegularExpression
        cStyleMultiBegin(
        QStringLiteral("^\\/\\*(?<identifier>[:=~%]|(\\s*?TRANSLATOR))\\s+(?<comment>.*)$"));

    static const QRegularExpression isSpace(QStringLiteral("\\s+"));
    static const QRegularExpression idefix(
        QStringLiteral("^\\/\\*(?<identifier>[:=~%]|(\\s*?TRANSLATOR))"));

    bool save = false;
    bool sawStarPrefix = false;
    bool sourceIdentifier = false;

    int storeLine = store->lupdateLocationLine;
    int lineExtra =  storeLine - 1;

    QString comment, identifier;
    for (auto line : commentLines) {
        line = line.trimmed();
        lineExtra++;
        if (!sawStarPrefix) {
            if (line.startsWith(QStringLiteral("//"))) {
                // Process C++ style comment.
                save = LupdatePrivate::capture(cppStyle, line, &identifier, &comment);
                storeLine = lineExtra;
            } else if (line.startsWith(QLatin1String("/*")) && line.endsWith(QLatin1String("*/"))) {
                // Process C style comment on a single line.
                storeLine = lineExtra;
                save = LupdatePrivate::capture(cStyleSingle, line, &identifier, &comment);
            } else if (line.startsWith(QLatin1String("/*"))) {
                storeLine = lineExtra;
                sawStarPrefix = true; // Start processing a multi line C style comment.

                auto result = idefix.match(line);
                if (!result.hasMatch())
                    continue; // No identifier found.
                identifier = result.captured(QLatin1String("identifier"));

                // The line is not just opening, try grab the comment.
                 if (line.size() > (identifier.size() + 3))
                    LupdatePrivate::capture(cStyleMultiBegin, line, &identifier, &comment);
                sourceIdentifier = (identifier == QLatin1String("%"));
            }
        } else {
            if (line.endsWith(QLatin1String("*/"))) {
                sawStarPrefix = false; // Finished processing a multi line C style comment.
                line = line.remove(QLatin1String("*/")).trimmed(); // Still there can be something.
            }

            if (sourceIdentifier) {
                line = LupdatePrivate::cleanQuote(line.toStdString(),
                    LupdatePrivate::QuoteCompulsary::Left);
            }

            if (!line.isEmpty() && !comment.isEmpty() && !sourceIdentifier)
                comment.append(QLatin1Char(' '));

            comment += line;
            save = !sawStarPrefix && !comment.isEmpty();
        }
        if (!save)
            continue;

        // To avoid processing the non TRANSLATOR comments when setInfoFromRawComment in called
        // from processIsolatedComments
        if (!store->funcName.isEmpty()) {
            if (identifier == QStringLiteral(":")) {
                if (!store->lupdateExtraComment.isEmpty())
                    store->lupdateExtraComment.append(QLatin1Char(' '));
                store->lupdateExtraComment += comment;
            } else if (identifier == QStringLiteral("=")) {
                if (!store->lupdateIdMetaData.isEmpty())
                    store->lupdateIdMetaData.append(QLatin1Char(' '));
                store->lupdateIdMetaData = comment; // Only the last one is to be picked up.
            } else if (identifier == QStringLiteral("~")) {
                auto first = comment.section(isSpace, 0, 0);
                auto second = comment.mid(first.size()).trimmed();
                if (!second.isEmpty())
                    store->lupdateAllMagicMetaData.insert(first, second);
            } else if (identifier == QLatin1String("%")) {
                store->lupdateSourceWhenId += comment;
            }
        } else if (identifier.trimmed() == QStringLiteral("TRANSLATOR")) {
            // separate the comment in two in order to get the context
            // then save it as a new entry in m_stores.
            // reminder: TRANSLATOR comments are isolated comments not linked to any tr call
            qCDebug(lcClang) << "Comment = " << comment;
            TranslationRelatedStore newStore;
            // need a funcName name in order to get handeled in fillTranslator
            newStore.funcName = QStringLiteral("TRANSLATOR");
            auto index = comment.indexOf(QStringLiteral(" "));
            if (index >= 0) {
                newStore.contextArg = comment.left(index).trimmed();
                newStore.lupdateComment = comment.mid(index).trimmed();
            }
            newStore.lupdateLocationFile = store->lupdateLocationFile;
            newStore.lupdateLocationLine = storeLine;
            newStore.locationCol = 0;
            newStore.printStore();
            m_trCalls.emplace_back(std::move(newStore));
        }

        save = false;
        comment.clear();
        identifier.clear();
    }
}

void LupdateVisitor::processPreprocessorCalls()
{
    QString inputFile = toQt(m_inputFile);
    for (const auto &store : m_stores->Preprocessor) {
        if (store.lupdateInputFile == inputFile)
            processPreprocessorCall(store);
    }

    if (m_qDeclareTrMacroAll.size() > 0 || m_noopTranslationMacroAll.size() > 0)
        m_macro = true;
}

void LupdateVisitor::processPreprocessorCall(TranslationRelatedStore store)
{
    // To get the comments around the macros
    const std::vector<QString> rawComments = rawCommentsFromSourceLocation(store
        .callLocation(m_context->getSourceManager()));
    // to pick up the raw comments in the files collected from the preprocessing.
    for (const auto &rawComment : rawComments)
        setInfoFromRawComment(rawComment, &store);

    // Processing the isolated comments (TRANSLATOR) in the files included in the main input file.
    if (store.callType.contains(QStringLiteral("InclusionDirective"))) {
        auto &sourceMgr = m_context->getSourceManager();
        const clang::FileID file = sourceMgr.getDecomposedLoc(store.callLocation(sourceMgr)).first;
        processIsolatedComments(file);
        return;
    }

    if (store.isValid()) {
        if (store.funcName.contains(QStringLiteral("Q_DECLARE_TR_FUNCTIONS")))
            m_qDeclareTrMacroAll.emplace_back(std::move(store));
        else
            m_noopTranslationMacroAll.emplace_back(std::move(store));
        store.printStore();
    }
}

bool LupdateVisitor::VisitNamedDecl(clang::NamedDecl *namedDeclaration)
{
    if (!m_macro)
        return true;
    auto fullLocation = m_context->getFullLoc(namedDeclaration->getBeginLoc());
    if (!fullLocation.isValid() || !fullLocation.getFileEntry())
        return true;

    if (!LupdatePrivate::isFileSignificant(fullLocation.getFileEntry()->getName().str()))
        return true;

    qCDebug(lcClang) << "NamedDecl Name:   " << QString::fromStdString(namedDeclaration->getQualifiedNameAsString());
    qCDebug(lcClang) << "NamedDecl source: " << QString::fromStdString(namedDeclaration->getSourceRange().printToString(
        m_context->getSourceManager()));
    // Checks if there is a macro located within the range of this NamedDeclaration
    // in order to find a context for the macro
    findContextForTranslationStoresFromPP(namedDeclaration);
    return true;
}

void LupdateVisitor::findContextForTranslationStoresFromPP(clang::NamedDecl *namedDeclaration)
{
    qCDebug(lcClang) << "=================findContextForTranslationStoresFromPP===================";
    qCDebug(lcClang) << "m_noopTranslationMacroAll " << m_noopTranslationMacroAll.size();
    qCDebug(lcClang) << "m_qDeclareTrMacroAll      " << m_qDeclareTrMacroAll.size();
    clang::SourceManager &sm = m_context->getSourceManager();

    // Looking for NOOP context only in the input file
    // because we are not interested in the NOOP from all related file
    // Once QT_TR_NOOP are gone this step can be removes because the only
    // QT_...NOOP left will have an context as argument
    for (TranslationRelatedStore &store : m_noopTranslationMacroAll) {
        if (!store.contextArg.isEmpty())
            continue;
        clang::SourceLocation sourceLoc = store.callLocation(sm);
        if (!sourceLoc.isValid())
            continue;
        if (LupdatePrivate::isPointWithin(namedDeclaration->getSourceRange(), sourceLoc, sm)) {

            store.contextRetrieved = LupdatePrivate::contextForNoopMacro(namedDeclaration, sm);
            qCDebug(lcClang) << "------------------------------------------NOOP Macro in range ---";
            qCDebug(lcClang) << "Range " << QString::fromStdString(namedDeclaration->getSourceRange().printToString(sm));
            qCDebug(lcClang) << "Point " << QString::fromStdString(sourceLoc.printToString(sm));
            qCDebug(lcClang) << "=========== Visit Named Declaration =============================";
            qCDebug(lcClang) << " Declaration Location    " <<
                QString::fromStdString(namedDeclaration->getSourceRange().printToString(sm));
            qCDebug(lcClang) << " Macro       Location                                 "
                << QString::fromStdString(sourceLoc.printToString(sm));
            qCDebug(lcClang) << " Context namedDeclaration->getQualifiedNameAsString() "
                << QString::fromStdString(namedDeclaration->getQualifiedNameAsString());
            qCDebug(lcClang) << " Context LupdatePrivate::contextForNoopMacro          "
                << store.contextRetrieved;
            qCDebug(lcClang) << " Context Retrieved       " << store.contextRetrieved;
            qCDebug(lcClang) << "=================================================================";
            store.printStore();
        }
    }

    for (TranslationRelatedStore &store : m_qDeclareTrMacroAll) {
        clang::SourceLocation sourceLoc = store.callLocation(sm);
        if (!sourceLoc.isValid())
            continue;
        if (LupdatePrivate::isPointWithin(namedDeclaration->getSourceRange(), sourceLoc, sm)) {
            store.contextRetrieved = QString::fromStdString(
                namedDeclaration->getQualifiedNameAsString());
            qCDebug(lcClang) << "------------------------------------------DECL Macro in range ---";
            qCDebug(lcClang) << "Range " << QString::fromStdString(namedDeclaration->getSourceRange().printToString(sm));
            qCDebug(lcClang) << "Point " << QString::fromStdString(sourceLoc.printToString(sm));
            qCDebug(lcClang) << "=========== Visit Named Declaration =============================";
            qCDebug(lcClang) << " Declaration Location    " <<
                QString::fromStdString(namedDeclaration->getSourceRange().printToString(sm));
            qCDebug(lcClang) << " Macro       Location                                 "
                << QString::fromStdString(sourceLoc.printToString(sm));
            qCDebug(lcClang) << " Context namedDeclaration->getQualifiedNameAsString() "
                << store.contextRetrieved;
            qCDebug(lcClang) << " Context Retrieved       " << store.contextRetrieved;
            qCDebug(lcClang) << "=================================================================";
            store.printStore();
        }
    }
}

void LupdateVisitor::generateOutput()
{
    qCDebug(lcClang) << "=================generateOutput============================";
    m_noopTranslationMacroAll.erase(std::remove_if(m_noopTranslationMacroAll.begin(),
        m_noopTranslationMacroAll.end(), [](const TranslationRelatedStore &store) {
        // Macros not located in the currently visited file are missing context (and it's normal),
        // so an output is only generated for macros present in the currently visited file.
        // If context could not be found, it is warned against in ClangCppParser::collectMessages
        // (where it is possible to order the warnings and print them consistantly)
        if (!LupdatePrivate::isFileSignificant(store.lupdateLocationFile.toStdString()))
            return true;
        return false;
      }), m_noopTranslationMacroAll.end());

    m_stores->QNoopTranlsationWithContext.emplace_bulk(std::move(m_noopTranslationMacroAll));

    m_qDeclareTrMacroAll.erase(std::remove_if(m_qDeclareTrMacroAll.begin(),
          m_qDeclareTrMacroAll.end(), [](const TranslationRelatedStore &store) {
              // only fill if a context has been retrieved in the file we're currently visiting
              return store.contextRetrieved.isEmpty();
      }), m_qDeclareTrMacroAll.end());
    m_stores->QDeclareTrWithContext.emplace_bulk(std::move(m_qDeclareTrMacroAll));

    processIsolatedComments();
    m_stores->AST.emplace_bulk(std::move(m_trCalls));
}

QT_END_NAMESPACE
