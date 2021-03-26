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

#ifndef CLANG_TOOL_AST_READER_H
#define CLANG_TOOL_AST_READER_H

#include "cpp_clang.h"
#if defined(Q_CC_MSVC)
# pragma warning(push)
# pragma warning(disable: 4100)
# pragma warning(disable: 4146)
# pragma warning(disable: 4267)
# pragma warning(disable: 4624)
#endif

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#if defined(Q_CC_MSVC)
# pragma warning(pop)
#endif

#include <iostream>
#include <memory>

QT_BEGIN_NAMESPACE

class Translator;

class LupdateVisitor : public clang::RecursiveASTVisitor<LupdateVisitor>
{
public:
    explicit LupdateVisitor(clang::ASTContext *context, Stores *stores)
        : m_context(context)
        , m_stores(stores)
    {
        m_inputFile = m_context->getSourceManager().getFileEntryForID(
            m_context->getSourceManager().getMainFileID())->getName();
    }

    bool VisitCallExpr(clang::CallExpr *callExpression);
    void processPreprocessorCalls();
    bool VisitNamedDecl(clang::NamedDecl *namedDeclaration);
    void findContextForTranslationStoresFromPP(clang::NamedDecl *namedDeclaration);
    void generateOuput();

private:
    std::vector<QString> rawCommentsForCallExpr(const clang::CallExpr *callExpr) const;
    std::vector<QString> rawCommentsFromSourceLocation(clang::SourceLocation sourceLocation) const;

    void setInfoFromRawComment(const QString &commentString, TranslationRelatedStore *store);

    void processPreprocessorCall(TranslationRelatedStore store);
    void processIsolatedComments();

    clang::ASTContext *m_context = nullptr;
    std::string m_inputFile;

    Stores *m_stores = nullptr;

    TranslationStores m_trCalls;
    TranslationStores m_qDeclareTrMacroAll;
    TranslationStores m_noopTranslationMacroAll;
    bool m_macro = false;
};

class LupdateASTConsumer : public clang::ASTConsumer
{
public:
    explicit LupdateASTConsumer(clang::ASTContext *context, Stores *stores)
        : m_visitor(context, stores)
    {}

    // This method is called when the ASTs for entire translation unit have been
    // parsed.
    void HandleTranslationUnit(clang::ASTContext &context) override
    {
        m_visitor.processPreprocessorCalls();
        bool traverse = m_visitor.TraverseAST(context);
        qCDebug(lcClang) << "TraverseAST: " << traverse;
        m_visitor.generateOuput();
    }

private:
    LupdateVisitor m_visitor;
};

class LupdateFrontendAction : public clang::ASTFrontendAction
{
public:
    LupdateFrontendAction(Stores *stores)
        : m_stores(stores)
    {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &compiler, llvm::StringRef /* inFile */) override
    {
        auto consumer = new LupdateASTConsumer(&compiler.getASTContext(), m_stores);
        return std::unique_ptr<clang::ASTConsumer>(consumer);
    }

private:
    Stores *m_stores = nullptr;
};

class LupdateToolActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    LupdateToolActionFactory(Stores *stores)
        : m_stores(stores)
    {}

#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<LupdateFrontendAction>(m_stores);
    }
#else
    clang::FrontendAction *create() override
    {
        return new LupdateFrontendAction(m_stores);
    }
#endif

private:
    Stores *m_stores = nullptr;
};

QT_END_NAMESPACE

#endif
