// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "csshighlighter_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

CssHighlighter::CssHighlighter(const CssHighlightColors &colors,
                               QTextDocument *document)
    : QSyntaxHighlighter(document), m_colors(colors)
{
}

void CssHighlighter::highlightBlock(const QString& text)
{
    enum Token { ALNUM, LBRACE, RBRACE, COLON, SEMICOLON, COMMA, QUOTE, SLASH, STAR };
    static const int transitions[10][9] = {
        { Selector, Property, Selector, Pseudo,    Property, Selector, Quote, MaybeComment, Selector }, // Selector
        { Property, Property, Selector, Value,     Property, Property, Quote, MaybeComment, Property }, // Property
        { Value,    Property, Selector, Value,     Property, Value,    Quote, MaybeComment, Value }, // Value
        { Pseudo1, Property, Selector, Pseudo2,    Selector, Selector, Quote, MaybeComment, Pseudo }, // Pseudo
        { Pseudo1, Property, Selector, Pseudo,    Selector, Selector, Quote, MaybeComment, Pseudo1 }, // Pseudo1
        { Pseudo2, Property, Selector, Pseudo,    Selector, Selector, Quote, MaybeComment, Pseudo2 }, // Pseudo2
        { Quote,    Quote,    Quote,    Quote,     Quote,    Quote,   -1, Quote, Quote }, // Quote
        { -1, -1, -1, -1, -1, -1, -1, -1, Comment }, // MaybeComment
        { Comment, Comment, Comment, Comment, Comment, Comment, Comment, Comment, MaybeCommentEnd }, // Comment
        { Comment, Comment, Comment, Comment, Comment, Comment, Comment, -1, MaybeCommentEnd } // MaybeCommentEnd
    };

    qsizetype lastIndex = 0;
    bool lastWasSlash = false;
    int state = previousBlockState(), save_state;
    if (state == -1) {
        // As long as the text is empty, leave the state undetermined
        if (text.isEmpty()) {
            setCurrentBlockState(-1);
            return;
        }
        // The initial state is based on the precense of a : and the absense of a {.
        // This is because Qt style sheets support both a full stylesheet as well as
        // an inline form with just properties.
        state = save_state = (text.indexOf(u':') > -1 &&
                              text.indexOf(u'{') == -1) ? Property : Selector;
    } else {
        save_state = state>>16;
        state &= 0x00ff;
    }

    if (state == MaybeCommentEnd) {
        state = Comment;
    } else if (state == MaybeComment) {
        state = save_state;
    }

    for (qsizetype i = 0; i < text.size(); ++i) {
        int token = ALNUM;
        const QChar c = text.at(i);
        const char a = c.toLatin1();

        if (state == Quote) {
            if (a == '\\') {
                lastWasSlash = true;
            } else {
                if (a == '\"' && !lastWasSlash) {
                    token = QUOTE;
                }
                lastWasSlash = false;
            }
        } else {
            switch (a) {
            case '{': token = LBRACE; break;
            case '}': token = RBRACE; break;
            case ':': token = COLON; break;
            case ';': token = SEMICOLON; break;
            case ',': token = COMMA; break;
            case '\"': token = QUOTE; break;
            case '/': token = SLASH; break;
            case '*': token = STAR; break;
            default: break;
            }
        }

        int new_state = transitions[state][token];

        if (new_state != state) {
            bool include_token = new_state == MaybeCommentEnd || (state == MaybeCommentEnd && new_state!= Comment)
                                 || state == Quote;
            highlight(text, lastIndex, i-lastIndex+include_token, state);

            if (new_state == Comment) {
                lastIndex = i-1; // include the slash and star
            } else {
                lastIndex = i + ((token == ALNUM || new_state == Quote) ? 0 : 1);
            }
        }

        if (new_state == -1) {
            state = save_state;
        } else if (state <= Pseudo2) {
            save_state = state;
            state = new_state;
        } else {
            state = new_state;
        }
    }

    highlight(text, lastIndex, text.size() - lastIndex, state);
    setCurrentBlockState(state + (save_state<<16));
}

void CssHighlighter::highlight(const QString &text, int start, int length, int state)
{
    if (start >= text.size() || length <= 0)
        return;

    QTextCharFormat format;

    switch (state) {
    case Selector:
        setFormat(start, length, m_colors.selector);
        break;
    case Property:
        setFormat(start, length, m_colors.property);
        break;
    case Value:
        setFormat(start, length, m_colors.value);
        break;
    case Pseudo1:
        setFormat(start, length, m_colors.pseudo1);
        break;
    case Pseudo2:
        setFormat(start, length, m_colors.pseudo2);
        break;
    case Quote:
        setFormat(start, length, m_colors.quote);
        break;
    case Comment:
    case MaybeCommentEnd:
        format.setForeground(m_colors.comment);
        setFormat(start, length, format);
        break;
    default:
        break;
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
