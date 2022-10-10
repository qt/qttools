// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qtextstream.h>
#include <QtWidgets/qtextedit.h>

#include "htmlhighlighter_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

HtmlHighlighter::HtmlHighlighter(QTextEdit *textEdit)
    : QSyntaxHighlighter(textEdit->document())
{
    QTextCharFormat entityFormat;
    entityFormat.setForeground(Qt::red);
    setFormatFor(Entity, entityFormat);

    QTextCharFormat tagFormat;
    tagFormat.setForeground(Qt::darkMagenta);
    tagFormat.setFontWeight(QFont::Bold);
    setFormatFor(Tag, tagFormat);

    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::gray);
    commentFormat.setFontItalic(true);
    setFormatFor(Comment, commentFormat);

    QTextCharFormat attributeFormat;
    attributeFormat.setForeground(Qt::black);
    attributeFormat.setFontWeight(QFont::Bold);
    setFormatFor(Attribute, attributeFormat);

    QTextCharFormat valueFormat;
    valueFormat.setForeground(Qt::blue);
    setFormatFor(Value, valueFormat);
}

void HtmlHighlighter::setFormatFor(Construct construct,
                                   const QTextCharFormat &format)
{
    m_formats[construct] = format;
    rehighlight();
}

void HtmlHighlighter::highlightBlock(const QString &text)
{
    static const QLatin1Char tab = QLatin1Char('\t');
    static const QLatin1Char space = QLatin1Char(' ');
    static const QLatin1Char amp = QLatin1Char('&');
    static const QLatin1Char startTag = QLatin1Char('<');
    static const QLatin1Char endTag = QLatin1Char('>');
    static const QLatin1Char quot = QLatin1Char('"');
    static const QLatin1Char apos = QLatin1Char('\'');
    static const QLatin1Char semicolon = QLatin1Char(';');
    static const QLatin1Char equals = QLatin1Char('=');
    static const QLatin1String startComment("<!--");
    static const QLatin1String endComment("-->");
    static const QLatin1String endElement("/>");

    int state = previousBlockState();
    qsizetype len = text.size();
    qsizetype start = 0;
    qsizetype pos = 0;

    while (pos < len) {
        switch (state) {
        case NormalState:
        default:
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == startTag) {
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
                }
                if (ch == amp) {
                    start = pos;
                    while (pos < len && text.at(pos++) != semicolon)
                        ;
                    setFormat(start, pos - start,
                              m_formats[Entity]);
                } else {
                    // No tag, comment or entity started, continue...
                    ++pos;
                }
            }
            break;
        case InComment:
            start = pos;
            for ( ; pos < len; ++pos) {
                if (text.mid(pos, 3) == endComment) {
                    pos += 3;
                    state = NormalState;
                    break;
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

} // namespace qdesigner_internal

QT_END_NAMESPACE
