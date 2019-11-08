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

#include "lupdatepreprocessoraction.h"

#include <clang/Lex/MacroArgs.h>
#include <clang/Basic/TokenKinds.h>

QT_BEGIN_NAMESPACE

void LupdatePPCallbacks::MacroExpands(const clang::Token &token,
    const clang::MacroDefinition &macroDefinition, clang::SourceRange sourceRange,
    const clang::MacroArgs *macroArgs)
{
    const auto &sm = m_preprocessor.getSourceManager();
    llvm::StringRef fileName = sm.getFilename(sourceRange.getBegin());
    if (fileName != m_inputFile)
        return;

    const QString funcName = QString::fromStdString(m_preprocessor.getSpelling(token));
    switch (trFunctionAliasManager.trFunctionByName(funcName)) {
    default:
        return;
    case TrFunctionAliasManager::Function_Q_DECLARE_TR_FUNCTIONS:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
    case TrFunctionAliasManager::Function_QT_TRID_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
        qCDebug(lcClang) << "MacroExpands: Function name:" << funcName;
        break;
    }

    TranslationRelatedStore store;
    store.callType = QStringLiteral("MacroExpands");
    store.funcName = funcName;
    store.lupdateLocationFile = QString::fromStdString(fileName);
    store.lupdateLocationLine = sm.getExpansionLineNumber(sourceRange.getBegin());
    store.locationCol = sm.getExpansionColumnNumber(sourceRange.getBegin());

    if (macroArgs) {
        std::vector<QString> arguments(macroArgs->getNumMacroArguments());
        for (unsigned i = 0; i < macroArgs->getNumMacroArguments(); i++) {
            auto preExpArguments = const_cast<clang::MacroArgs*>(macroArgs)->getPreExpArgument(i,
                m_preprocessor);
            QString temp;
            bool errorArgument = false;
            for (const auto &preExpArgument : preExpArguments) {
                const auto kind = preExpArgument.getKind();
                switch (trFunctionAliasManager.trFunctionByName(funcName)) {
                default:
                    break;
                case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
                case TrFunctionAliasManager::Function_QT_TRID_NOOP:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
                    if (!clang::tok::isStringLiteral(kind))
                       errorArgument = true;
                    break;
                }
                if (errorArgument)
                    break;
                if (clang::tok::isStringLiteral(kind))
                    temp += LupdatePrivate::cleanQuote(m_preprocessor.getSpelling(preExpArgument));
                else
                    temp += QString::fromStdString(m_preprocessor.getSpelling(preExpArgument));
            }
            arguments[i] = temp;
        }
        storeMacroArguments(arguments, &store);
    }
    if (store.isValid())
        m_stores.push_back(store);
}

void LupdatePPCallbacks::storeMacroArguments(const std::vector<QString> &args,
    TranslationRelatedStore *store)
{
    switch (trFunctionAliasManager.trFunctionByName(store->funcName)) {
    // only one argument: the context with no "
    case TrFunctionAliasManager::Function_Q_DECLARE_TR_FUNCTIONS:
        if (args.size() == 1)
            store->contextArg = args[0];
        break;
    // two arguments: the context and the source
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
        if (args.size() >= 2) {
            store->contextArg = args[0];
            store->lupdateSource = args[1];
        }
        if (args.size() == 3)
            store->lupdateComment = args[2];
        break;
    // only one argument (?) the message Id
    case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
        Q_FALLTHROUGH();
    case TrFunctionAliasManager::Function_qtTrId:
    case TrFunctionAliasManager::Function_QT_TRID_NOOP:
        if (args.size() == 1)
            store->lupdateId = args[0];
        break;
    }
}

QT_END_NAMESPACE
