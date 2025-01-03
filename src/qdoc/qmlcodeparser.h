// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLCODEPARSER_H
#define QMLCODEPARSER_H

#include "codeparser.h"

#include <QtCore/qset.h>

#ifndef QT_NO_DECLARATIVE
#    include <private/qqmljsengine_p.h>
#    include <private/qqmljslexer_p.h>
#    include <private/qqmljsparser_p.h>
#endif

QT_BEGIN_NAMESPACE

class Node;
class QString;

class QmlCodeParser : public CodeParser
{
public:
    QmlCodeParser();
    ~QmlCodeParser() override = default;

    void initializeParser() override;
    void terminateParser() override;
    QString language() override;
    QStringList sourceFileNameFilter() override;
    void parseSourceFile(const Location &location, const QString &filePath) override;

#ifndef QT_NO_DECLARATIVE
    /* Copied from src/declarative/qml/qdeclarativescriptparser.cpp */
    void extractPragmas(QString &script);
#endif

protected:
    const QSet<QString> &topicCommands();

private:
#ifndef QT_NO_DECLARATIVE
    QQmlJS::Engine m_engine {};
    QQmlJS::Lexer *m_lexer { nullptr };
    QQmlJS::Parser *m_parser { nullptr };
#endif
};

QT_END_NAMESPACE

#endif
