// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qtextstream.h>
#include <QtWidgets/qtextedit.h>

#include "htmlhighlighter_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

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
    static const QChar tab = u'\t';
    static const QChar space = u' ';

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
                if (ch == u'<') {
                    if (QStringView{text}.sliced(pos).startsWith("<!--"_L1)) {
                        state = InComment;
                    } else {
                        state = InTag;
                        start = pos;
                        while (pos < len && text.at(pos) != space
                               && text.at(pos) != u'>'
                               && text.at(pos) != tab
                               && !QStringView{text}.sliced(pos).startsWith("/>"_L1)) {
                            ++pos;
                        }
                        if (QStringView{text}.sliced(pos).startsWith("/>"_L1))
                            ++pos;
                        setFormat(start, pos - start,
                                  m_formats[Tag]);
                        break;
                    }
                    break;
                }
                if (ch == u'&') {
                    start = pos;
                    while (pos < len && text.at(pos++) != u';')
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
                if (QStringView{text}.sliced(pos).startsWith("-->"_L1)) {
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
                    if (ch == '\''_L1 || ch == u'"') {
                        quote = ch;
                    } else if (ch == u'>') {
                        ++pos;
                        setFormat(start, pos - start, m_formats[Tag]);
                        state = NormalState;
                        break;
                    } else if (QStringView{text}.sliced(pos).startsWith("/>"_L1)) {
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
                               && text.at(pos) != u'=')
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
