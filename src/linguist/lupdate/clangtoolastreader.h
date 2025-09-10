// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CLANG_TOOL_AST_READER_H
#define CLANG_TOOL_AST_READER_H

#include "cpp_clang.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4100)
QT_WARNING_DISABLE_MSVC(4146)
QT_WARNING_DISABLE_MSVC(4267)
QT_WARNING_DISABLE_MSVC(4624)
QT_WARNING_DISABLE_GCC("-Wnonnull")

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

QT_WARNING_POP

#include <iostream>
#include <memory>

QT_BEGIN_NAMESPACE

class Translator;

class LupdateVisitor : public clang::RecursiveASTVisitor<LupdateVisitor>
{
public:
#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(14,0,0))
    explicit LupdateVisitor(clang::ASTContext *context,
                            clang::Preprocessor *preprocessor, Stores *stores)
        : m_context(context)
        , m_preprocessor(preprocessor)
        , m_stores(stores)
#else
    explicit LupdateVisitor(clang::ASTContext *context, Stores *stores)
        : m_context(context)
        , m_stores(stores)
#endif
    {
        m_inputFile = m_context->getSourceManager().getFileEntryRefForID(
            m_context->getSourceManager().getMainFileID())->getName();
    }

    bool VisitCallExpr(clang::CallExpr *callExpression);
    void processPreprocessorCalls();
    bool VisitNamedDecl(clang::NamedDecl *namedDeclaration);
    void findContextForTranslationStoresFromPP(clang::NamedDecl *namedDeclaration);
    void generateOutput();

private:
    std::vector<QString> rawCommentsForCallExpr(const clang::CallExpr *callExpr) const;
    std::vector<QString> rawCommentsFromSourceLocation(clang::SourceLocation sourceLocation) const;

    void setInfoFromRawComment(const QString &commentString, TranslationRelatedStore *store);

    void processPreprocessorCall(TranslationRelatedStore store);
    void processIsolatedComments();
    void processIsolatedComments(const clang::FileID file);

    clang::ASTContext *m_context = nullptr;
#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(14,0,0))
    clang::Preprocessor *m_preprocessor = nullptr;
#endif
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
#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(14,0,0))
    explicit LupdateASTConsumer(clang::ASTContext *context, clang::Preprocessor *preprocessor,
                                Stores *stores)
        : m_visitor(context, preprocessor, stores)
#else
    explicit LupdateASTConsumer(clang::ASTContext *context, Stores *stores)
        : m_visitor(context, stores)
#endif
    {}

    // This method is called when the ASTs for entire translation unit have been
    // parsed.
    void HandleTranslationUnit(clang::ASTContext &context) override
    {
        m_visitor.processPreprocessorCalls();
        bool traverse = m_visitor.TraverseAST(context);
        qCDebug(lcClang) << "TraverseAST: " << traverse;
        m_visitor.generateOutput();
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
        #if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(14,0,0))
            auto consumer = new LupdateASTConsumer(&compiler.getASTContext(),
                                                   &compiler.getPreprocessor(), m_stores);
        #else
            auto consumer = new LupdateASTConsumer(&compiler.getASTContext(), m_stores);
        #endif
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
