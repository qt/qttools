/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

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
