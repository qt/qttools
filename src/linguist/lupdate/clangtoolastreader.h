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

#include "lupdate.h"
#include "translator.h"
#include "translatormessage.h"

#include <QtCore/qloggingcategory.h>

#if defined(Q_CC_MSVC)
# pragma warning(disable: 4100)
# pragma warning(disable: 4146)
# pragma warning(disable: 4267)
# pragma warning(disable: 4624)
#endif

#include <llvm/ADT/APInt.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#if defined(Q_CC_MSVC)
# pragma warning(default: 4100)
# pragma warning(default: 4146)
# pragma warning(default: 4267)
# pragma warning(default: 4624)
#endif

#include <memory>
#include <string>
#include <utility>
#include <vector>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcClang)

#define LUPDATE_CLANG_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define LUPDATE_CLANG_VERSION LUPDATE_CLANG_VERSION_CHECK(LUPDATE_CLANG_VERSION_MAJOR, \
    LUPDATE_CLANG_VERSION_MINOR, LUPDATE_CLANG_VERSION_PATCH)

class LupdateVisitor : public clang::RecursiveASTVisitor<LupdateVisitor>
{
public:
    explicit LupdateVisitor(clang::ASTContext *Context, Translator *tor)
        : m_tor(tor),
          m_context(Context)
    {}

private:
    clang::ASTContext *m_context { nullptr };
    Translator *m_tor { nullptr };
};

class LupdateASTConsumer : public clang::ASTConsumer
{
public:
    explicit LupdateASTConsumer(clang::ASTContext *Context, Translator *tor)
        : m_visitor(Context, tor)
    {}

    //This method is called when the ASTs for entire translation unit have been parsed.
    void HandleTranslationUnit(clang::ASTContext &Context) override
    {
        bool traverse = m_visitor.TraverseAST(Context);
        qCDebug(lcClang) << "TraverseAST: " << traverse << "\n";
    }

private:
    LupdateVisitor m_visitor;
};

class LupdateFrontendAction : public clang::ASTFrontendAction
{
public:
    LupdateFrontendAction(Translator *tor)
        : m_tor(tor)
    {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &Compiler, llvm::StringRef InFile) override
    {
        LupdateASTConsumer *consumer = new LupdateASTConsumer(&Compiler.getASTContext(), m_tor);
        return std::unique_ptr<clang::ASTConsumer>(consumer);
    }

private:
    Translator *m_tor { nullptr };
};

class LupdateToolActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    LupdateToolActionFactory(Translator *tor)
        : m_tor(tor)
    {}

#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<LupdateFrontendAction>(m_tor);
    }
#else
    clang::FrontendAction *create() override
    {
        return new LupdateFrontendAction(m_tor);
    }
#endif

private:
    Translator *m_tor { nullptr };
};

QT_END_NAMESPACE

#endif
