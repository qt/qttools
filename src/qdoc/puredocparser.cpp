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
void PureDocParser::parseSourceFile(const Location &location, const QString &filePath)
{
    QFile in(filePath);
    m_currentFile = filePath;
    if (!in.open(QIODevice::ReadOnly)) {
        location.error(
                QStringLiteral("Can't open source file '%1' (%2)").arg(filePath, strerror(errno)));
        m_currentFile.clear();
        return;
    }

    Location fileLocation(filePath);
    Tokenizer fileTokenizer(fileLocation, in);
    m_tokenizer = &fileTokenizer;
    m_token = m_tokenizer->getToken();

    /*
      The set of open namespaces is cleared before parsing
      each source file. The word "source" here means cpp file.
     */
    m_qdb->clearOpenNamespaces();

    processQdocComments();
    in.close();
    m_currentFile.clear();
}

/*!
  This is called by parseSourceFile() to do the actual parsing
  and tree building. It only processes qdoc comments. It skips
  everything else.
 */
bool PureDocParser::processQdocComments()
{
    const QSet<QString> &commands = topicCommands() + metaCommands();

    while (m_token != Tok_Eoi) {
        if (m_token == Tok_Doc) {
            QString comment = m_tokenizer->lexeme(); // returns an entire qdoc comment.
            Location start_loc(m_tokenizer->location());
            m_token = m_tokenizer->getToken();

            Doc::trimCStyleComment(start_loc, comment);
            Location end_loc(m_tokenizer->location());

            // Doc constructor parses the comment.
            Doc doc(start_loc, end_loc, comment, commands, topicCommands());
            const TopicList &topics = doc.topicsUsed();
            if (topics.isEmpty()) {
                doc.location().warning(QStringLiteral("This qdoc comment contains no topic command "
                                                      "(e.g., '\\%1', '\\%2').")
                                               .arg(COMMAND_MODULE, COMMAND_PAGE));
                continue;
            }
            if (hasTooManyTopics(doc))
                continue;

            DocList docs;
            NodeList nodes;
            QString topic = topics[0].m_topic;

            processTopicArgs(doc, topic, nodes, docs);
            processMetaCommands(nodes, docs);
        } else {
            m_token = m_tokenizer->getToken();
        }
    }
    return true;
}

QT_END_NAMESPACE
