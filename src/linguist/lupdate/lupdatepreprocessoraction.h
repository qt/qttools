/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef LUPDATEPREPROCESSORACTION_H
#define LUPDATEPREPROCESSORACTION_H

#include "cpp_clang.h"
#include "synchronized.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4100)
QT_WARNING_DISABLE_MSVC(4146)
QT_WARNING_DISABLE_MSVC(4267)
QT_WARNING_DISABLE_MSVC(4624)
QT_WARNING_DISABLE_GCC("-Wnonnull")

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

QT_WARNING_POP

#include <memory>

QT_BEGIN_NAMESPACE

class LupdatePPCallbacks : public clang::PPCallbacks
{
public:
    LupdatePPCallbacks(WriteSynchronizedRef<TranslationRelatedStore> *stores, clang::Preprocessor &pp)
        : m_preprocessor(pp)
        , m_stores(stores)
    {
        const auto &sm = m_preprocessor.getSourceManager();
        m_inputFile = sm.getFileEntryForID(sm.getMainFileID())->getName();
    }

    ~LupdatePPCallbacks() override
    {
        m_stores->emplace_bulk(std::move(m_ppStores));
    }

private:
    void MacroExpands(const clang::Token &token, const clang::MacroDefinition &macroDefinition,
        clang::SourceRange sourceRange, const clang::MacroArgs *macroArgs) override;

    void storeMacroArguments(const std::vector<QString> &args, TranslationRelatedStore *store);

    void SourceRangeSkipped(clang::SourceRange sourceRange, clang::SourceLocation endifLoc) override;
    void InclusionDirective(clang::SourceLocation /*hashLoc*/, const clang::Token &/*includeTok*/,
                            clang::StringRef /*fileName*/, bool /*isAngled*/,
                            clang::CharSourceRange /*filenameRange*/,
#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(15,0,0))
                            const clang::Optional<clang::FileEntryRef> file,
#else
                            const clang::FileEntry *file,
#endif
                            clang::StringRef /*searchPath*/, clang::StringRef /*relativePath*/,
                            const clang::Module */*imported*/,
                            clang::SrcMgr::CharacteristicKind /*fileType*/) override;

    std::string m_inputFile;
    clang::Preprocessor &m_preprocessor;

    TranslationStores m_ppStores;
    WriteSynchronizedRef<TranslationRelatedStore> *m_stores { nullptr };
};

class LupdatePreprocessorAction : public clang::PreprocessOnlyAction
{
public:
    LupdatePreprocessorAction(WriteSynchronizedRef<TranslationRelatedStore> *stores)
        : m_stores(stores)
    {}

private:
    void ExecuteAction() override
    {
        auto &preprocessor = getCompilerInstance().getPreprocessor();
        preprocessor.SetSuppressIncludeNotFoundError(true);
        auto callbacks = new LupdatePPCallbacks(m_stores, preprocessor);
        preprocessor.addPPCallbacks(std::unique_ptr<clang::PPCallbacks>(callbacks));

        clang::PreprocessOnlyAction::ExecuteAction();
    }

private:
    WriteSynchronizedRef<TranslationRelatedStore> *m_stores { nullptr };
};

class LupdatePreprocessorActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    explicit LupdatePreprocessorActionFactory(WriteSynchronizedRef<TranslationRelatedStore> *stores)
        : m_stores(stores)
    {}

#if (LUPDATE_CLANG_VERSION >= LUPDATE_CLANG_VERSION_CHECK(10,0,0))
    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<LupdatePreprocessorAction>(m_stores);
    }
#else
    clang::FrontendAction *create() override
    {
        return new LupdatePreprocessorAction(m_stores);
    }
#endif

private:
    WriteSynchronizedRef<TranslationRelatedStore> *m_stores { nullptr };
};

QT_END_NAMESPACE

#endif
