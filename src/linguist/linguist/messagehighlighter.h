/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef MESSAGEHIGHLIGHTER_H
#define MESSAGEHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

QT_BEGIN_NAMESPACE

class QTextEdit;

/* Message highlighter based on HtmlSyntaxHighlighter from designer */
class MessageHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    MessageHighlighter(QTextEdit *textEdit);

protected:
    void highlightBlock(const QString &text) override;

private:
    enum Construct {
        Entity,
        Tag,
        Comment,
        Attribute,
        Value,
        Accelerator, // "Open &File"
        Variable,    // "Opening %1"
        LastConstruct = Variable
    };

    enum State {
        NormalState = -1,
        InComment,
        InTag
    };

    QTextCharFormat m_formats[LastConstruct + 1];
};

QT_END_NAMESPACE

#endif // MESSAGEHIGHLIGHTER_H
