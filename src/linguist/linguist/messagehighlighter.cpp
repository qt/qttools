// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "messagehighlighter.h"

#include "globals.h"

#include <QtCore/QTextStream>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

MessageHighlighter::MessageHighlighter(QTextEdit *textEdit)
    : QSyntaxHighlighter(textEdit->document())
{
    adjustColors();
}

void MessageHighlighter::highlightBlock(const QString &text)
{
    static const QLatin1Char tab = QLatin1Char('\t');
    static const QLatin1Char space = QLatin1Char(' ');
    static const QLatin1Char amp = QLatin1Char('&');
    static const QLatin1Char endTag = QLatin1Char('>');
    static const QLatin1Char quot = QLatin1Char('"');
    static const QLatin1Char apos = QLatin1Char('\'');
    static const QLatin1Char semicolon = QLatin1Char(';');
    static const QLatin1Char equals = QLatin1Char('=');
    static const QLatin1Char percent = QLatin1Char('%');
    static const QLatin1String startComment = QLatin1String("<!--");
    static const QLatin1String endComment = QLatin1String("-->");
    static const QLatin1String endElement = QLatin1String("/>");

    int state = previousBlockState();
    int len = text.size();
    int start = 0;
    int pos = 0;

    while (pos < len) {
        switch (state) {
        case NormalState:
        default:
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == QLatin1Char('<')) {
                    if (text.mid(pos, 4) == startComment) {
                        state = InComment;
                    } else {
                        state = InTag;
                        start = pos;
                        while (pos < len && text.at(pos) != space
                               && text.at(pos) != endTag
                               && text.at(pos) != tab
                               && text.mid(pos, 2) != endElement)
                            ++pos;
                        if (text.mid(pos, 2) == endElement)
                            ++pos;
                        setFormat(start, pos - start,
                                  m_formats[Tag]);
                        break;
                    }
                    break;
                } else if (ch == amp && pos + 1 < len) {
                    // Default is Accelerator
                    if (text.at(pos + 1).isLetterOrNumber())
                        setFormat(pos + 1, 1, m_formats[Accelerator]);

                    // When a semicolon follows assume an Entity
                    start = pos;
                    ch = text.at(++pos);
                    while (pos + 1 < len && ch != semicolon && ch.isLetterOrNumber())
                        ch = text.at(++pos);
                    if (ch == semicolon)
                        setFormat(start, pos - start + 1, m_formats[Entity]);
                } else if (ch == percent) {
                    start = pos;
                    // %[1-9]*
                    for (++pos; pos < len && text.at(pos).isDigit(); ++pos) {}
                    // %n
                    if (pos < len && pos == start + 1 && text.at(pos) == QLatin1Char('n'))
                        ++pos;
                    setFormat(start, pos - start, m_formats[Variable]);
                } else {
                    // No tag, comment or entity started, continue...
                    ++pos;
                }
            }
            break;
        case InComment:
            start = pos;
            while (pos < len) {
                if (text.mid(pos, 3) == endComment) {
                    pos += 3;
                    state = NormalState;
                    break;
                } else {
                    ++pos;
                }
            }
            setFormat(start, pos - start, m_formats[Comment]);
            break;
        case InTag:
            QChar quote = QChar::Null;
            while (pos < len) {
                QChar ch = text.at(pos);
                if (quote.isNull()) {
                    start = pos;
                    if (ch == apos || ch == quot) {
                        quote = ch;
                    } else if (ch == endTag) {
                        ++pos;
                        setFormat(start, pos - start, m_formats[Tag]);
                        state = NormalState;
                        break;
                    } else if (text.mid(pos, 2) == endElement) {
                        pos += 2;
                        setFormat(start, pos - start, m_formats[Tag]);
                        state = NormalState;
                        break;
                    } else if (ch != space && text.at(pos) != tab) {
                        // Tag not ending, not a quote and no whitespace, so
                        // we must be dealing with an attribute.
                        ++pos;
                        while (pos < len && text.at(pos) != space
                               && text.at(pos) != tab
                               && text.at(pos) != equals)
                            ++pos;
                        setFormat(start, pos - start, m_formats[Attribute]);
                        start = pos;
                    }
                } else if (ch == quote) {
                    quote = QChar::Null;

                    // Anything quoted is a value
                    setFormat(start, pos - start, m_formats[Value]);
                }
                ++pos;
            }
            break;
        }
    }
    setCurrentBlockState(state);
}

void MessageHighlighter::adjustColors()
{
    QTextCharFormat entityFormat;
    QTextCharFormat tagFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat attributeFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat acceleratorFormat;
    QTextCharFormat variableFormat;

    if (isDarkMode()) {
        entityFormat.setForeground(Qt::red);
        tagFormat.setForeground(QColor(Qt::darkMagenta).lighter());
        commentFormat.setForeground(Qt::gray);
        attributeFormat.setForeground(QColor(Qt::darkGray).lighter());
        valueFormat.setForeground(QColor(Qt::darkGreen).lighter());
        variableFormat.setForeground(QColor(Qt::darkGreen).lighter());
    } else {
        entityFormat.setForeground(Qt::red);
        tagFormat.setForeground(Qt::darkMagenta);
        commentFormat.setForeground(Qt::gray);
        attributeFormat.setForeground(Qt::black);
        valueFormat.setForeground(Qt::darkGreen);
        variableFormat.setForeground(Qt::darkGreen);
    }

    commentFormat.setFontItalic(true);
    attributeFormat.setFontItalic(true);
    acceleratorFormat.setFontUnderline(true);

    m_formats[Entity] = entityFormat;
    m_formats[Tag] = tagFormat;
    m_formats[Comment] = commentFormat;
    m_formats[Attribute] = attributeFormat;
    m_formats[Value] = valueFormat;
    m_formats[Accelerator] = acceleratorFormat;
    m_formats[Variable] = variableFormat;

    rehighlight();
}

QT_END_NAMESPACE
