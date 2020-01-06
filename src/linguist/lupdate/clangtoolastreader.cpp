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

#include "clangtoolastreader.h"
#include "translator.h"

QT_BEGIN_NAMESPACE

namespace LupdatePrivate
{
    /*
        Retrieves the context for the NOOP macros using the context of
        the NamedDeclaration within which the Macro is.
        The context is stripped of the function or method part as it used not to be retrieved
        in the previous cpp parser.
    */
    QString contextForNoopMacro(clang::NamedDecl *namedDecl)
    {
        QStringList context;
        const clang::DeclContext *decl = namedDecl->getDeclContext();
        while (decl) {
            if (clang::isa<clang::NamedDecl>(decl) && !decl->isFunctionOrMethod()) {
                if (const auto *namespaceDecl = clang::dyn_cast<clang::NamespaceDecl>(decl)) {
                    context.prepend(namespaceDecl->isAnonymousNamespace()
                        ? QStringLiteral("(anonymous namespace)")
                        : QString::fromStdString(namespaceDecl->getDeclName().getAsString()));
                } else if (const auto *recordDecl = clang::dyn_cast<clang::RecordDecl>(decl)) {
                    static const QString anonymous = QStringLiteral("(anonymous %1)");
                    context.prepend(recordDecl->getIdentifier()
                        ? QString::fromStdString(recordDecl->getDeclName().getAsString())
                        : anonymous.arg(QLatin1String(recordDecl->getKindName().data())));
                }
            }
            decl = decl->getParent();
        }
        return context.join(QStringLiteral("::"));
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

    // Checking that the CallExpression is from the input file we're interested in
    std::string fileName;
    if (const auto fileEntry = fullLocation.getFileEntry())
        fileName = fileEntry->getName();
    if (fileName != m_inputFile)
        return true;

    clang::FunctionDecl *func = callExpression->getDirectCallee();
    if (!func)
        return true;
    clang::QualType q = callExpression->getType();
    if (!q.getTypePtrOrNull())
        return true;
    const std::string funcName = func->getNameInfo().getAsString();
    // Only continue if the function a translation function (TODO: deal with alias function...)
    if (funcName != "tr" && funcName != "qtTrId" && funcName != "translate" && funcName != "trUtf8")
        return true;

    qCDebug(lcClang) << "************************** VisitCallExpr ****************";
    // Retrieving the information needed to fill the lupdate translator.
    // Function independent retrieve
    TranslationRelatedStore store;
    store.callType = QStringLiteral("ASTRead_CallExpr");
    store.funcName = QString::fromStdString(funcName);
    store.lupdateLocationFile = QString::fromStdString(fileName);
    store.lupdateLocationLine = fullLocation.getSpellingLineNumber();
    store.contextRetrieved = LupdatePrivate::contextForFunctionDecl(func, funcName);

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
    switch (trFunctionAliasManager.trFunctionByName(QString::fromStdString(funcName))) {
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

/*
    Retrieve the comments not associated with tr calls.
*/
void LupdateVisitor::processIsolatedComments()
{
    qCDebug(lcClang) << "==== processIsolatedComments ====";
    auto &sourceMgr = m_context->getSourceManager();

#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
    const clang::FileID file = sourceMgr.getMainFileID();
    const auto commentsInThisFile = m_context->getRawCommentList().getCommentsInFile(file);
    if (!commentsInThisFile)
        return;

    std::vector<clang::RawComment *> tmp;
    for (const auto &commentInFile : *commentsInThisFile)
        tmp.emplace_back(commentInFile.second);
    clang::ArrayRef<clang::RawComment *> rawComments = tmp;
#else
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
        if (sourceMgr.getFilename(rawComment->getBeginLoc()).str() != m_inputFile)
            continue;
        // Comments not separated by an empty line will be part of the same Raw comments
        // Each one needs to be saved with its line number.
        // The store is used here only to pass this information.
        TranslationRelatedStore store;
        store.lupdateLocationLine = sourceMgr.getPresumedLoc(rawComment->getBeginLoc()).getLine();
        qCDebug(lcClang) << " raw Comment : \n"
            << QString::fromStdString(rawComment->getRawText(sourceMgr));
        setInfoFromRawComment(QString::fromStdString(rawComment->getRawText(sourceMgr)), &store);
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
    const auto commentsInThisFile = m_context->getRawCommentList().getCommentsInFile(file);
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
    const clang::BeforeThanCompare<clang::RawComment> compareSourceLocation(sourceMgr);
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
            << sourceMgr.getFilename(sourceLocation);
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
                << (*comment)->getRawText(sourceMgr) << "' is ignored, return.";
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
                << (*comment)->getRawText(sourceMgr) << "' is ignored, continue.";
            continue; // if there is a comment on the previous line it should be picked up
        }

        // There should be no other translation function between comment and declaration.
        if (LupdatePrivate::trFunctionPresent(text)) {
            qCDebug(lcClang) << "Found another translation function between comment and "
                "declaration, break.";
            break;
        }

        retrievedRawComments.emplace(retrievedRawComments.begin(),
            QString::fromStdString((*comment)->getRawText(sourceMgr)));
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
            newStore.lupdateLocationFile = QString::fromStdString(m_inputFile);
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
    m_macro = (m_stores->Preprocessor.size() > 0);
    for (const auto &store : m_stores->Preprocessor)
        processPreprocessorCall(store);
}

void LupdateVisitor::processPreprocessorCall(TranslationRelatedStore store)
{
    const std::vector<QString> rawComments = rawCommentsFromSourceLocation(store
        .callLocation(m_context->getSourceManager()));
    for (const auto &rawComment : rawComments)
        setInfoFromRawComment(rawComment, &store);

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

    if (fullLocation.getFileEntry()->getName() != m_inputFile)
        return true;

    qCDebug(lcClang) << "NamedDecl Name:   " << namedDeclaration->getQualifiedNameAsString();
    qCDebug(lcClang) << "NamedDecl source: " << namedDeclaration->getSourceRange().printToString(
        m_context->getSourceManager());
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
            /*
                void N3::C1::C12::C121::f2()
                {
                    const char test_NOOP[] = QT_TR_NOOP("A QT_TR_NOOP N3::C1::C13");
                }
                In such case namedDeclaration->getQualifiedNameAsString() will give only
                test_NOOP as context.
                This is why the following function is needed
            */
            store.contextRetrievedTempNOOP = LupdatePrivate::contextForNoopMacro(namedDeclaration);
            qCDebug(lcClang) << "------------------------------------------NOOP Macro in range ---";
            qCDebug(lcClang) << "Range " << namedDeclaration->getSourceRange().printToString(sm);
            qCDebug(lcClang) << "Point " << sourceLoc.printToString(sm);
            qCDebug(lcClang) << "=========== Visit Named Declaration =============================";
            qCDebug(lcClang) << " Declaration Location    " <<
                namedDeclaration->getSourceRange().printToString(sm);
            qCDebug(lcClang) << " Macro       Location                                 "
                << sourceLoc.printToString(sm);
            qCDebug(lcClang) << " Context namedDeclaration->getQualifiedNameAsString() "
                << namedDeclaration->getQualifiedNameAsString();
            qCDebug(lcClang) << " Context LupdatePrivate::contextForNoopMacro          "
                << store.contextRetrievedTempNOOP;
            qCDebug(lcClang) << " Context Retrieved       " << store.contextRetrievedTempNOOP;
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
            qCDebug(lcClang) << "Range " << namedDeclaration->getSourceRange().printToString(sm);
            qCDebug(lcClang) << "Point " << sourceLoc.printToString(sm);
            qCDebug(lcClang) << "=========== Visit Named Declaration =============================";
            qCDebug(lcClang) << " Declaration Location    " <<
                namedDeclaration->getSourceRange().printToString(sm);
            qCDebug(lcClang) << " Macro       Location                                 "
                << sourceLoc.printToString(sm);
            qCDebug(lcClang) << " Context namedDeclaration->getQualifiedNameAsString() "
                << store.contextRetrieved;
            qCDebug(lcClang) << " Context LupdatePrivate::contextForNoopMacro          "
                << LupdatePrivate::contextForNoopMacro(namedDeclaration);
            qCDebug(lcClang) << " Context Retrieved       " << store.contextRetrieved;
            qCDebug(lcClang) << "=================================================================";
            store.printStore();
        }
    }
}

void LupdateVisitor::generateOuput()
{
    qCDebug(lcClang) << "=================m_trCallserateOuput============================";
    m_noopTranslationMacroAll.erase(std::remove_if(m_noopTranslationMacroAll.begin(),
          m_noopTranslationMacroAll.end(), [](const TranslationRelatedStore &store) {
              // only fill if a context has been retrieved in the file we're currently visiting
              return store.contextRetrievedTempNOOP.isEmpty() && store.contextArg.isEmpty();
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
