// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef CLANGCODEPARSER_H
#define CLANGCODEPARSER_H

#include "cppcodeparser.h"

#include "config.h"

#include <QtCore/qtemporarydir.h>
#include <QtCore/QStringList>

typedef struct CXTranslationUnitImpl *CXTranslationUnit;

QT_BEGIN_NAMESPACE

struct ParsedCppFileIR {
    std::vector<UntiedDocumentation> untied;
    std::vector<TiedDocumentation> tied;
};

class ClangCodeParser : public CodeParser
{
public:
    ClangCodeParser(Config&, const std::vector<QByteArray>& include_paths, const QList<QByteArray>& defines);
    ~ClangCodeParser() override = default;

    void initializeParser() override {}
    void terminateParser() override {}
    QString language() override;
    QStringList sourceFileNameFilter() override;
    void parseSourceFile(const Location &, const QString &, CppCodeParser&) override {}
    ParsedCppFileIR parse_cpp_file(const QString &filePath);
    void buildPCH(QString module_header);
    Node *parseFnArg(const Location &location, const QString &fnSignature, const QString &idTag, QStringList context);

private:
    void getDefaultArgs();
    void getMoreArgs();

    std::set<Config::HeaderFilePath> m_allHeaders {}; // file name->path
    const std::vector<QByteArray>& m_includePaths;
    QScopedPointer<QTemporaryDir> m_pchFileDir {};
    QByteArray m_pchName {};
    QList<QByteArray> m_defines {};
    std::vector<const char *> m_args {};
    QList<QByteArray> m_moreArgs {};
    QStringList m_namespaceScope {};
    QByteArray s_fn;
};

QT_END_NAMESPACE

#endif
