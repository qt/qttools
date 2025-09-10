// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CLANGCODEPARSER_H
#define CLANGCODEPARSER_H

#include "codeparser.h"
#include "parsererror.h"
#include "config.h"

#include <QtCore/qtemporarydir.h>
#include <QtCore/QStringList>

#include <optional>

typedef struct CXTranslationUnitImpl *CXTranslationUnit;

class CppCodeParser;

QT_BEGIN_NAMESPACE

struct ParsedCppFileIR {
    std::vector<UntiedDocumentation> untied;
    std::vector<TiedDocumentation> tied;
};

struct PCHFile {
    QTemporaryDir dir;
    QByteArray name;
};

std::optional<PCHFile> buildPCH(
    QDocDatabase* qdb,
    QString module_header,
    const std::set<Config::HeaderFilePath>& all_headers,
    const std::vector<QByteArray>& include_paths,
    const QList<QByteArray>& defines
);

struct FnCommandParser {
    FnCommandParser(
        QDocDatabase* qdb,
        const std::set<Config::HeaderFilePath>& all_headers,
        const QList<QByteArray>& defines,
        std::optional<std::reference_wrapper<const PCHFile>> pch
    ) : m_qdb{qdb},
        m_allHeaders{all_headers},
        m_defines{defines},
        m_args{},
        m_pch{pch}
    {}

    std::variant<Node*, FnMatchError> operator()(
        const Location &location,
        const QString &fnSignature,
        const QString &idTag,
        QStringList context
   );

private:
    QDocDatabase* m_qdb;
    const std::set<Config::HeaderFilePath>& m_allHeaders; // file name->path
    QList<QByteArray> m_defines {};
    std::vector<const char *> m_args {};
    std::optional<std::reference_wrapper<const PCHFile>> m_pch;
};

class ClangCodeParser
{
public:
    ClangCodeParser(
        QDocDatabase* qdb,
        Config&,
        const std::vector<QByteArray>& include_paths,
        const QList<QByteArray>& defines,
        std::optional<std::reference_wrapper<const PCHFile>> pch
    );

    ParsedCppFileIR parse_cpp_file(const QString &filePath);

private:
    QDocDatabase* m_qdb{};
    std::set<Config::HeaderFilePath> m_allHeaders {}; // file name->path
    const std::vector<QByteArray>& m_includePaths;
    QList<QByteArray> m_defines {};
    std::vector<const char *> m_args {};
    QStringList m_namespaceScope {};
    QByteArray s_fn;
    std::optional<std::reference_wrapper<const PCHFile>> m_pch;
};

QT_END_NAMESPACE

#endif
