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

inline QDebug operator<<(QDebug out, const std::string& str)
{
    out << QString::fromStdString(str);
    return out;
}
Q_DECLARE_LOGGING_CATEGORY(lcClang)

#define LUPDATE_CLANG_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define LUPDATE_CLANG_VERSION LUPDATE_CLANG_VERSION_CHECK(LUPDATE_CLANG_VERSION_MAJOR, \
    LUPDATE_CLANG_VERSION_MINOR, LUPDATE_CLANG_VERSION_PATCH)

// Local storage of translation information (information from the AST and linguist side)
struct TranslationRelatedStore
{
    QString callType;
    QString rawCode;
    QString funcName;
    qint64 locationCol = -1;
    QString contextArg;
    QString contextRetrieved;
    QString lupdateSource;
    QString lupdateLocationFile;
    qint64 lupdateLocationLine = -1;
    QString lupdateId;
    QString lupdateSourceWhenId;
    QString lupdateIdMetaData;
    QString lupdateMagicMetaData;
    QHash<QString, QString> lupdateAllMagicMetaData;
    /*
     * NOTE: lupdate
     * //~ meta1 toto
     * //~ meta2 titi
     * //~ meta2 tata
     * =>
     * <extra-meta1> toto </extra-meta1>
     * <extra-meta2> tata </extra-meta2>
     * => titi is lost. For the moment we do as lupdate cpp.cpp does
     */
    QString lupdateComment;      //  the one as argument of the tr function
    QString lupdateExtraComment; //  those  //:
    QString lupdatePlural;

    bool isValid() const
    {
        return !lupdateLocationFile.isEmpty() && (lupdateLocationLine > -1) && (locationCol > -1);
    }

    void printStore() const
    {
        qCDebug(lcClang) << "------------------ Printing Store----------------------------------\n";
        qCDebug(lcClang) << "callType            : " << callType << "\n"
            << "rawCode             : \n" << rawCode << "\n"
            << "funcName            : " << funcName << "\n"
            << "LocationCol         : " << locationCol << "\n"
            << "contextArg          : " << contextArg << "\n"
            << "contextRetrieved    : " << contextRetrieved << "\n"
            << "lupdateSource       : " << lupdateSource << "\n"
            << "lupdateLocationFile : " << lupdateLocationFile << "\n"
            << "lupdateLocationLine : " << lupdateLocationLine << "\n"
            << "lupdateId           : " << lupdateId << "\n"
            << "lupdateIdMetaData   : " << lupdateIdMetaData << "\n"
            << "lupdateMagicMetaData: " << lupdateMagicMetaData << "\n"
            << "lupdateComment      : " << lupdateComment << "\n"
            << "lupdateExtraComment : " << lupdateExtraComment << "\n"
            << "lupdatePlural       : " << lupdatePlural << "\n";
        qCDebug(lcClang) << "-------------------------------------------------------------------\n";
    }
};

class LupdateVisitor : public clang::RecursiveASTVisitor<LupdateVisitor>
{
public:
    explicit LupdateVisitor(clang::ASTContext *context, Translator *tor)
        : m_context(context),
          m_tor(tor)
    {
        m_inputFile = m_context->getSourceManager().getFileEntryForID(
            m_context->getSourceManager().getMainFileID())->getName().rsplit(
            llvm::StringRef("/")).second;
    }

    bool VisitCallExpr(clang::CallExpr *callExpression);
    void fillTranslator();

private:
    std::vector<QString> rawCommentsForCallExpr(const clang::CallExpr *callExpr) const;
    std::vector<QString> rawCommentsFromSourceLocation(clang::SourceLocation sourceLocation) const;

    void setInfoFromRawComment(const QString &commentString, TranslationRelatedStore *store);

    // Functions used to fill the linguist translator
    // local store -> linguist Translator
    void fillTranslator(TranslationRelatedStore store);
    TranslatorMessage fillTranslatorMessage(const TranslationRelatedStore &store,
        bool forcePlural, bool isID = false);
    void handleTr(const TranslationRelatedStore &store, bool forcePlural);
    void handleTrId(const TranslationRelatedStore &store, bool forcePlural);
    void handleTranslate(const TranslationRelatedStore &store, bool forcePlural);

    clang::ASTContext *m_context { nullptr };
    Translator *m_tor { nullptr };
    std::string m_inputFile;

    // Local store for the lupdate translation related info
    std::vector<TranslationRelatedStore> m_translationStoresFromAST;
};

class LupdateASTConsumer : public clang::ASTConsumer
{
public:
    explicit LupdateASTConsumer(clang::ASTContext *context, Translator *tor)
        : m_visitor(context, tor)
    {}

    //This method is called when the ASTs for entire translation unit have been parsed.
    void HandleTranslationUnit(clang::ASTContext &context) override
    {
        bool traverse = m_visitor.TraverseAST(context);
        qCDebug(lcClang) << "TraverseAST: " << traverse << "\n";
        m_visitor.fillTranslator();
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
        clang::CompilerInstance &compiler, llvm::StringRef /* inFile */) override
    {
        LupdateASTConsumer *consumer = new LupdateASTConsumer(&compiler.getASTContext(), m_tor);
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
