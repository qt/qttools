/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
****************************************************************************/

#include "puredocparser.h"

#include "qdocdatabase.h"
#include "tokenizer.h"

#include <errno.h>

QT_BEGIN_NAMESPACE

PureDocParser *PureDocParser::pureParser_ = nullptr;

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
    currentFile_ = filePath;
    if (!in.open(QIODevice::ReadOnly)) {
        location.error(tr("Can't open source file '%1' (%2)").arg(filePath).arg(strerror(errno)));
        currentFile_.clear();
        return;
    }

    Location fileLocation(filePath);
    Tokenizer fileTokenizer(fileLocation, in);
    tokenizer_ = &fileTokenizer;
    tok_ = tokenizer_->getToken();

    /*
      The set of open namespaces is cleared before parsing
      each source file. The word "source" here means cpp file.
     */
    qdb_->clearOpenNamespaces();

    processQdocComments();
    in.close();
    currentFile_.clear();
}

/*!
  This is called by parseSourceFile() to do the actual parsing
  and tree building. It only processes qdoc comments. It skips
  everything else.
 */
bool PureDocParser::processQdocComments()
{
    const QSet<QString> &commands = topicCommands() + metaCommands();

    while (tok_ != Tok_Eoi) {
        if (tok_ == Tok_Doc) {
            QString comment = tokenizer_->lexeme(); // returns an entire qdoc comment.
            Location start_loc(tokenizer_->location());
            tok_ = tokenizer_->getToken();

            Doc::trimCStyleComment(start_loc, comment);
            Location end_loc(tokenizer_->location());

            // Doc constructor parses the comment.
            Doc doc(start_loc, end_loc, comment, commands, topicCommands());
            const TopicList &topics = doc.topicsUsed();
            if (topics.isEmpty()) {
                doc.location().warning(tr("This qdoc comment contains no topic command "
                                          "(e.g., '\\%1', '\\%2').")
                                               .arg(COMMAND_MODULE)
                                               .arg(COMMAND_PAGE));
                continue;
            }
            if (hasTooManyTopics(doc))
                continue;

            DocList docs;
            NodeList nodes;
            QString topic = topics[0].topic;

            processTopicArgs(doc, topic, nodes, docs);
            processMetaCommands(nodes, docs);
        } else {
            tok_ = tokenizer_->getToken();
        }
    }
    return true;
}

QT_END_NAMESPACE
