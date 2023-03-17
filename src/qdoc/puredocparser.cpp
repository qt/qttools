// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "puredocparser.h"

#include "qdocdatabase.h"
#include "tokenizer.h"

#include <cerrno>

QT_BEGIN_NAMESPACE

/*!
  Returns a list of the kinds of files that the pure doc
  parser is meant to parse. The elements of the list are
  file suffixes.
 */
QStringList PureDocParser::sourceFileNameFilter()
{
    return QStringList() << "*.qdoc"
                         << "*.qtx"
                         << "*.qtt"
                         << "*.js";
}

/*!
  Parses the source file identified by \a filePath and adds its
  parsed contents to the database. The \a location is used for
  reporting errors.
 */
void PureDocParser::parseSourceFile(const Location &location, const QString &filePath, CppCodeParser& cpp_code_parser)
{
    QFile in(filePath);
    if (!in.open(QIODevice::ReadOnly)) {
        location.error(
                QStringLiteral("Can't open source file '%1' (%2)").arg(filePath, strerror(errno)));
        return;
    }

    /*
      The set of open namespaces is cleared before parsing
      each source file. The word "source" here means cpp file.
     */
    m_qdb->clearOpenNamespaces();

    processQdocComments(in, cpp_code_parser);
    in.close();
}

/*!
  This is called by parseSourceFile() to do the actual parsing
  and tree building. It only processes qdoc comments. It skips
  everything else.
 */
void PureDocParser::processQdocComments(QFile& input_file, CppCodeParser& cpp_code_parser)
{
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
        const TopicList &topics = doc.topicsUsed();
        if (topics.isEmpty()) {
            doc.location().warning(QStringLiteral("This qdoc comment contains no topic command "
                                                  "(e.g., '\\%1', '\\%2').")
                                           .arg(COMMAND_MODULE, COMMAND_PAGE));
            continue;
        }

        if (cpp_code_parser.hasTooManyTopics(doc))
            continue;

        DocList docs;
        NodeList nodes;
        QString topic = topics[0].m_topic;

        cpp_code_parser.processTopicArgs(doc, topic, nodes, docs);
        cpp_code_parser.processMetaCommands(nodes, docs);
    }
}

QT_END_NAMESPACE
