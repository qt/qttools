// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "clangcodeparser.h"
#include "puredocparser.h"

#include <variant>

#include <QString>
#include <QFileInfo>

struct CppSourceFile {};
struct CppHeaderSourceFile {};
struct QDocSourceFile {};
struct JsSourceFile {};
struct UnknownSourceFile {};

using SourceFileTag = std::variant<CppSourceFile, CppHeaderSourceFile, QDocSourceFile, JsSourceFile, UnknownSourceFile>;
using TaggedSourceFile = std::pair<QString, SourceFileTag>;

inline TaggedSourceFile tag_source_file(const QString& path) {
    static QStringList cpp_file_extensions{
        "c++", "cc", "cpp", "cxx", "mm"
    };
    static QStringList cpp_header_extensions{ "h", "h++", "hpp", "hxx" };
    static QStringList qdoc_file_extensions{ "qdoc" };
    static QStringList javascript_file_extensions{ "js" };

    QString extension{QFileInfo(path).suffix()};

    const bool inHeaders = Config::instance().get(CONFIG_DOCUMENTATIONINHEADERS).asBool();

    if (inHeaders && cpp_header_extensions.contains(extension)) {
        return TaggedSourceFile{ path, CppHeaderSourceFile{} };
    } else if (cpp_file_extensions.contains(extension)) {
        return TaggedSourceFile{ path, CppSourceFile{} };
    } else if (qdoc_file_extensions.contains(extension))
        return TaggedSourceFile{ path, QDocSourceFile{} };
    else if (javascript_file_extensions.contains(extension)) return TaggedSourceFile{path, JsSourceFile{}};

    return TaggedSourceFile{path, UnknownSourceFile{}};
}

class SourceFileParser {
public:
    struct ParseResult {
        std::vector<UntiedDocumentation> untied;
        std::vector<TiedDocumentation> tied;
    };

public:
    SourceFileParser(ClangCodeParser& clang_parser, PureDocParser& pure_parser)
        : cpp_file_parser(clang_parser),
          pure_file_parser(pure_parser)
    {}

    ParseResult operator()(const TaggedSourceFile& source) {
        if (std::holds_alternative<CppSourceFile>(source.second))
            return (*this)(source.first, std::get<CppSourceFile>(source.second));
        else if (std::holds_alternative<CppHeaderSourceFile>(source.second))
            return (*this)(source.first, std::get<CppHeaderSourceFile>(source.second));
        else if (std::holds_alternative<QDocSourceFile>(source.second))
            return (*this)(source.first, std::get<QDocSourceFile>(source.second));
        else if (std::holds_alternative<JsSourceFile>(source.second))
            return (*this)(source.first, std::get<JsSourceFile>(source.second));
        else if (std::holds_alternative<UnknownSourceFile>(source.second))
            return {{}, {}};

        Q_UNREACHABLE();
    }

private:
    ParseResult operator()(const QString& path, CppSourceFile) {
         auto [untied, tied] = cpp_file_parser.parse_cpp_file(path);

         return {untied, tied};
    }
    ParseResult operator()(const QString& path, CppHeaderSourceFile) {
        return (*this)(path, CppSourceFile{});
    }

    ParseResult operator()(const QString& path, QDocSourceFile) {
        return {pure_file_parser.parse_qdoc_file(path), {}};
    }

    ParseResult operator()(const QString& path, JsSourceFile) {
        return {pure_file_parser.parse_qdoc_file(path), {}};
    }

private:
    ClangCodeParser& cpp_file_parser;
    PureDocParser& pure_file_parser;
};
