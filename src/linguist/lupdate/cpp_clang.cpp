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

#include <translator.h>

QT_BEGIN_NAMESPACE

void ClangCppParser::loadCPP(Translator &translator, const QStringList &filenames,
    ConversionData &cd)
{
    // Going through the files to be parsed
    std::vector<std::string> sources;
    for (const QString &filename: filenames)
       sources.push_back(filename.toStdString());

    // The ClangTool is to be created and run from this function.

    // First we'll need an OptionParser
    // Then we'll create a ClangTool taking the OptionParser and the sources as argument

    // The translator to store the information from the parsing of the files.
    Translator *tor = new Translator();

    // TODO: set up clang tool for parsing
    qWarning("lupdate: Clang based C++ parser not implemented!");

    // TODO: remove this printing at a later point
    // Printing the translator (storage and manipulation of translation info from linguist module)
    if (qEnvironmentVariableIsSet("QT_LUPDATE_CLANG_DEBUG"))
        tor->dump();

    for (const TranslatorMessage &msg: tor->messages())
        translator.extend(msg, cd);

}
QT_END_NAMESPACE
