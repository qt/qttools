// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "location.h"

#include <QtCore/qfile.h>
#include <QtCore/qstack.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

/*
  Here come the C++ tokens we support.  The first part contains
  all-purpose tokens; then come keywords.

  If you add a keyword, make sure to modify the keyword array in
  tokenizer.cpp as well, and possibly adjust Tok_FirstKeyword and
  Tok_LastKeyword.
*/
enum {
    Tok_Eoi,
    Tok_Ampersand,
    Tok_Aster,
    Tok_Caret,
    Tok_LeftParen,
    Tok_RightParen,
    Tok_LeftParenAster,
    Tok_Equal,
    Tok_LeftBrace,
    Tok_RightBrace,
    Tok_Semicolon,
    Tok_Colon,
    Tok_LeftAngle,
    Tok_RightAngle,
    Tok_Comma,
    Tok_Ellipsis,
    Tok_Gulbrandsen,
    Tok_LeftBracket,
    Tok_RightBracket,
    Tok_Tilde,
    Tok_SomeOperator,
    Tok_Number,
    Tok_String,
    Tok_Doc,
    Tok_Comment,
    Tok_Ident,
    Tok_At,
    Tok_char,
    Tok_class,
    Tok_const,
    Tok_double,
    Tok_int,
    Tok_long,
    Tok_operator,
    Tok_short,
    Tok_signed,
    Tok_typename,
    Tok_unsigned,
    Tok_void,
    Tok_volatile,
    Tok_int64,
    Tok_QPrivateSignal,
    Tok_FirstKeyword = Tok_char,
    Tok_LastKeyword = Tok_QPrivateSignal
};

/*
  The Tokenizer class implements lexical analysis of C++ source
  files.

  Not every operator or keyword of C++ is recognized; only those
  that are interesting to us. Some Qt keywords or macros are also
  recognized.
*/

class Tokenizer
{
public:
    Tokenizer(const Location &loc, QByteArray in);
    Tokenizer(const Location &loc, QFile &file);

    ~Tokenizer();

    int getToken();
    void setParsingFnOrMacro(bool macro) { m_parsingMacro = macro; }

    [[nodiscard]] const Location &location() const { return m_tokLoc; }
    [[nodiscard]] QString previousLexeme() const;
    [[nodiscard]] QString lexeme() const;
    [[nodiscard]] QString version() const { return m_version; }
    [[nodiscard]] int parenDepth() const { return m_parenDepth; }
    [[nodiscard]] int bracketDepth() const { return m_bracketDepth; }

    static void initialize();
    static void terminate();
    static bool isTrue(const QString &condition);

private:
    void init();
    void start(const Location &loc);
    /*
     Represents the maximum amount of characters that a token can be composed
     of.

     When a token with more characters than the maximum amount is encountered, a
     warning is issued and parsing continues, discarding all characters from the
     currently parsed token that don't fit into the buffer.
    */
    enum { yyLexBufSize = 1048576 };

    int getch() { return m_pos == m_in.size() ? EOF : m_in[m_pos++]; }

    inline int getChar()
    {
        using namespace Qt::StringLiterals;

        if (m_ch == EOF)
            return EOF;
        if (m_lexLen < yyLexBufSize - 1) {
            m_lex[m_lexLen++] = (char)m_ch;
            m_lex[m_lexLen] = '\0';
        } else if (!token_too_long_warning_was_issued) {
            location().warning(
                u"The content is too long.\n"_s,
                u"The maximum amount of characters for this content is %1.\n"_s.arg(yyLexBufSize) +
                "Consider splitting it or reducing its size."
            );

            token_too_long_warning_was_issued = true;
        }
        m_curLoc.advance(QChar(m_ch));
        int ch = getch();
        if (ch == EOF)
            return EOF;
        // cast explicitly to make sure the value of ch
        // is in range [0..255] to avoid assert messages
        // when using debug CRT that checks its input.
        return int(uint(uchar(ch)));
    }

    int getTokenAfterPreprocessor();
    void pushSkipping(bool skip);
    bool popSkipping();

    Location m_tokLoc;
    Location m_curLoc;
    char *m_lexBuf1 { nullptr };
    char *m_lexBuf2 { nullptr };
    char *m_prevLex { nullptr };
    char *m_lex { nullptr };
    size_t m_lexLen {};
    QStack<bool> m_preprocessorSkipping;
    int m_numPreprocessorSkipping {};
    int m_braceDepth {};
    int m_parenDepth {};
    int m_bracketDepth {};
    int m_ch {};

    QString m_version {};
    bool m_parsingMacro {};

    // Used to ensure that the warning that is issued when a token is
    // too long to fit into our fixed sized buffer is not repeated for each
    // character of that token after the last saved one.
    // The flag is reset whenever a new token is requested, so as to allow
    // reporting all such tokens that are too long during a single execution.
    bool token_too_long_warning_was_issued{false};

protected:
    QByteArray m_in {};
    int m_pos {};
};

QT_END_NAMESPACE

#endif
