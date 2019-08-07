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

QT_END_NAMESPACE
