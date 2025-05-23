// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
