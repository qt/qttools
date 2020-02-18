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

#include "cpp_clang.h"
#include "clangtoolastreader.h"
#include "lupdatepreprocessoraction.h"
#include "translator.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Option/Option.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcClang, "qt.lupdate.clang");

// This is a way to add options related to the customized clang tool
// Needed as one of the arguments to create the OptionParser.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// Makes sure all the comments will be parsed and part of the AST
// Clang will run with the flag -fparse-all-comments
clang::tooling::ArgumentsAdjuster getClangArgumentAdjuster()
{
    return [](const clang::tooling::CommandLineArguments &args, llvm::StringRef /*unused*/) {
        clang::tooling::CommandLineArguments adjustedArgs;
        for (size_t i = 0, e = args.size(); i < e; ++i) {
            llvm::StringRef arg = args[i];
            // FIXME: Remove options that generate output.
            if (!arg.startswith("-fcolor-diagnostics") && !arg.startswith("-fdiagnostics-color"))
                adjustedArgs.push_back(args[i]);
        }
        adjustedArgs.push_back("-fparse-all-comments");
        adjustedArgs.push_back("-I");
        adjustedArgs.push_back(CLANG_RESOURCE_DIR);
        return adjustedArgs;
    };
}

void ClangCppParser::loadCPP(Translator &translator, const QStringList &filenames,
    ConversionData &cd)
{
    std::vector<std::string> sources;
    for (const QString &filename : filenames)
        sources.push_back(filename.toStdString());

    // The ClangTool is to be created and run from this function.

    int argc = 4;
    // NEED 2 empty one to start!!! otherwise: LLVM::ERROR
    const QByteArray jsonPath = cd.m_compileCommandsPath.toLocal8Bit();
    const char *argv[4] = { "", "", "-p", jsonPath.constData() };
    clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);

    clang::tooling::ClangTool tool(OptionsParser.getCompilations(), sources);
    tool.appendArgumentsAdjuster(getClangArgumentAdjuster());

    Stores stores;
    tool.run(new LupdatePreprocessorActionFactory(stores.Preprocessor));

    Translator *tor = new Translator();

    // A ClangTool needs a new FrontendAction for each translation unit it runs on
    // A Customized FrontendActionFactory is building a customized FrondendAction
    tool.run(new LupdateToolActionFactory(tor));

    if (QLoggingCategory("qt.lupdate.clang").isDebugEnabled())
        tor->dump();

    for (const TranslatorMessage &msg : tor->messages())
        translator.extend(msg, cd);
}

QT_END_NAMESPACE
