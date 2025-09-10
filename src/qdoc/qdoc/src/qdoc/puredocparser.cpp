// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "puredocparser.h"

#include "qdocdatabase.h"
#include "tokenizer.h"

#include <cerrno>

QT_BEGIN_NAMESPACE

/*!
  Parses the source file identified by \a filePath and adds its
  parsed contents to the database. The \a location is used for
  reporting errors.
 */
std::vector<UntiedDocumentation> PureDocParser::parse_qdoc_file(const QString &filePath)
{
    QFile in(filePath);
    if (!in.open(QIODevice::ReadOnly)) {
        location.error(
                QStringLiteral("Can't open source file '%1' (%2)").arg(filePath, strerror(errno)));
        return {};
    }

    return processQdocComments(in);
}

/*!
  This is called by parseSourceFile() to do the actual parsing
  and tree building. It only processes qdoc comments. It skips
  everything else.
 */
std::vector<UntiedDocumentation> PureDocParser::processQdocComments(QFile& input_file)
{
    std::vector<UntiedDocumentation> untied{};

    Tokenizer tokenizer(Location{input_file.fileName()}, input_file);

    const QSet<QString> &commands = CppCodeParser::topic_commands + CppCodeParser::meta_commands;

    int token = tokenizer.getToken();
    while (token != Tok_Eoi) {
        if (token != Tok_Doc) {
            token = tokenizer.getToken();
            continue;
        }
        QString comment = tokenizer.lexeme(); // returns an entire qdoc comment.
        Location start_loc(tokenizer.location());
        token = tokenizer.getToken();

        Doc::trimCStyleComment(start_loc, comment);
        Location end_loc(tokenizer.location());

        // Doc constructor parses the comment.
        Doc doc(start_loc, end_loc, comment, commands, CppCodeParser::topic_commands);
        if (doc.topicsUsed().isEmpty()) {
            doc.location().warning(QStringLiteral("This qdoc comment contains no topic command "
                                                  "(e.g., '\\%1', '\\%2').")
                                           .arg(COMMAND_MODULE, COMMAND_PAGE));
            continue;
        }

        if (hasTooManyTopics(doc))
            continue;

        untied.emplace_back(UntiedDocumentation{doc, QStringList()});
    }

    return untied;
}

QT_END_NAMESPACE
