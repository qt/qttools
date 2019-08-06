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

QT_BEGIN_NAMESPACE

namespace LupdatePrivate
{
    QString cleanContext(std::string &context, std::string &suffix)
    {
        return QString::fromStdString(context.substr(0, context.find(suffix, 0)));
    }

    // Cleaning quotes around the source and the comment quoteCompulsory: we
    // only want to retrieve what is inside the quotes and the text won't be
    // retrieved if quotes are not there if the quoteCompulsary = false simply
    // return the input, line by line with the spaces at the beginning of the
    // line removed not looking for real code comment
    QString cleanQuote(llvm::StringRef s, bool quoteCompulsory = true)
    {
        qCDebug(lcClang) << "==========================================text to clean " << s.str();
        if (s.empty())
            return QString::fromStdString(s);
        s = s.trim();
        if (!s.consume_front("\"") && quoteCompulsory)
            return {};
        if (!s.consume_back("\"") && quoteCompulsory)
            return {};
        return QString::fromStdString(s);
    }

    bool hasQuote(llvm::StringRef source)
    {
        return source.contains("\"");
    }

    bool trFunctionPresent(llvm::StringRef text)
    {
        /*
            UNARY_MACRO(qtTrId)
            UNARY_MACRO(tr)
            UNARY_MACRO(trUtf8)
            UNARY_MACRO(translate)
        */
        if (text.contains(llvm::StringRef("qtTrId(")))
            return true;
        if (text.contains(llvm::StringRef("tr(")))
            return true;
        if (text.contains(llvm::StringRef("trUtf8(")))
            return true;
        if (text.contains(llvm::StringRef("translate(")))
            return true;
        return false;
    }
}

// The visit functions are called automatically when the visitor TraverseAST function is called
// This is the function where the tr qtIdTr translate function are actually picked up
// In the AST, tr, qtIdTr and translate function are always part of a CallExpression.
bool LupdateVisitor::VisitCallExpr(clang::CallExpr *callExpression)
{
    clang::FullSourceLoc fullLocation = m_context->getFullLoc(callExpression->getBeginLoc());
    if (fullLocation.isInvalid() || !fullLocation.getFileEntry())
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
    llvm::StringRef fileName = fullLocation.getFileEntry()->getName();
    if (fileName.contains(llvm::StringRef("/")))
        fileName = fileName.rsplit(llvm::StringRef("/")).second;
    // Checking that the CallExpression is from the input file we're interested in
    if (fileName != m_inputFile)
        return true;

    qCDebug(lcClang) << "************************** VisitCallExpr ****************";
    // Retrieving the information needed to fill the lupdate translator.
    // Function independent retrieve
    TranslationRelatedStore store;
    store.callType = QString::fromLatin1("ASTRead_CallExpr");
    store.funcName = QString::fromStdString(funcName);
    store.lupdateLocationFile = QString::fromStdString(fullLocation.getFileEntry()->getName());
    store.lupdateLocationLine = fullLocation.getSpellingLineNumber();
    qCDebug(lcClang) << "CallType          : ASTRead_CallExpr\n";
    qCDebug(lcClang) << "Function name     : " << store.funcName << "\n";
    qCDebug(lcClang) << "File location     : " << store.lupdateLocationFile << "\n";
    qCDebug(lcClang) << "Line              : " << store.lupdateLocationLine << "\n";

    std::string context = func->getQualifiedNameAsString();
    std::string contextSuffix = "::";
    contextSuffix += funcName;
    store.contextRetrieved = LupdatePrivate::cleanContext(context, contextSuffix);
    qCDebug(lcClang) << "Context retrieved : "
        << store.contextRetrieved << "\n";

    // Here we gonna need to retrieve the comments around the function call
    // //: //* //~ Things like that
    const std::vector<QString> rawComments = rawCommentsForCallExpr(callExpression);
    for (const auto &rawComment : rawComments)
            qCDebug(lcClang) << "Raw comments     :" << rawComment << "\n";

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
        qCDebug(lcClang) << "Source      : " << store.lupdateSource << "\n";
        qCDebug(lcClang) << "Comment     : " << store.lupdateComment << "\n";
        qCDebug(lcClang) << "Plural      : " << store.lupdatePlural << "\n";
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
        qCDebug(lcClang) << "Context Arg : " << store.contextArg << "\n";
        qCDebug(lcClang) << "Source      : " << store.lupdateSource << "\n";
        qCDebug(lcClang) << "Comment     : " << store.lupdateComment << "\n";
        qCDebug(lcClang) << "Plural      : " << store.lupdatePlural << "\n";
        break;
    case TrFunctionAliasManager::Function_qtTrId:
        if (arguments.size() != 2 || !LupdatePrivate::hasQuote(arguments[0]))
            return true;
        store.lupdateId = LupdatePrivate::cleanQuote(arguments[0]);
        store.lupdatePlural = QString::fromStdString(arguments[1]);
        qCDebug(lcClang) << "ID          : " << store.lupdateId << "\n";
        qCDebug(lcClang) << "Plural      : " << store.lupdatePlural << "\n";
        break;
    }
    return true;
}

// Retrieve the comments associated with the CallExpression
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
    for (auto commentInFile : *commentsInThisFile)
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
        if (declLineNum == sourceMgr.getLineNumber(commentEndDecomp.first, commentEndDecomp.second)) {
            qCDebug(lcClang) << "Comment ends on same line as the declaration. Comment '"
                << (*comment)->getRawText(sourceMgr) << "' is ignored, continue.";
            continue;
        }

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

QT_END_NAMESPACE
