// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tokenizer.h"

#include "config.h"
#include "generator.h"

#include <QtCore/qfile.h>
#include <QtCore/qhash.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringconverter.h>

#include <cctype>
#include <cstring>
#include <utility>

QT_BEGIN_NAMESPACE

#define LANGUAGE_CPP "Cpp"

/* qmake ignore Q_OBJECT */

/*
  Keep in sync with tokenizer.h.
*/
static const char *kwords[] = { "char",
                                "class",
                                "const",
                                "double",
                                "enum",
                                "explicit",
                                "friend",
                                "inline",
                                "int",
                                "long",
                                "namespace",
                                "operator",
                                "private",
                                "protected",
                                "public",
                                "short",
                                "signals",
                                "signed",
                                "slots",
                                "static",
                                "struct",
                                "template",
                                "typedef",
                                "typename",
                                "union",
                                "unsigned",
                                "using",
                                "virtual",
                                "void",
                                "volatile",
                                "__int64",
                                "default",
                                "delete",
                                "final",
                                "override",
                                "Q_OBJECT",
                                "Q_OVERRIDE",
                                "Q_PROPERTY",
                                "Q_PRIVATE_PROPERTY",
                                "Q_DECLARE_SEQUENTIAL_ITERATOR",
                                "Q_DECLARE_MUTABLE_SEQUENTIAL_ITERATOR",
                                "Q_DECLARE_ASSOCIATIVE_ITERATOR",
                                "Q_DECLARE_MUTABLE_ASSOCIATIVE_ITERATOR",
                                "Q_DECLARE_FLAGS",
                                "Q_SIGNALS",
                                "Q_SLOTS",
                                "QT_COMPAT",
                                "QT_COMPAT_CONSTRUCTOR",
                                "QT_DEPRECATED",
                                "QT_MOC_COMPAT",
                                "QT_MODULE",
                                "QT3_SUPPORT",
                                "QT3_SUPPORT_CONSTRUCTOR",
                                "QT3_MOC_SUPPORT",
                                "QDOC_PROPERTY",
                                "QPrivateSignal" };

static const int KwordHashTableSize = 4096;
static int kwordHashTable[KwordHashTableSize];

static QHash<QByteArray, bool> *ignoredTokensAndDirectives = nullptr;

static QRegularExpression *comment = nullptr;
static QRegularExpression *versionX = nullptr;
static QRegularExpression *definedX = nullptr;

static QRegularExpression *defines = nullptr;
static QRegularExpression *falsehoods = nullptr;

static QStringDecoder sourceDecoder;

/*
  This function is a perfect hash function for the 37 keywords of C99
  (with a hash table size of 512). It should perform well on our
  Qt-enhanced C++ subset.
*/
static int hashKword(const char *s, int len)
{
    return (((uchar)s[0]) + (((uchar)s[2]) << 5) + (((uchar)s[len - 1]) << 3)) % KwordHashTableSize;
}

static void insertKwordIntoHash(const char *s, int number)
{
    int k = hashKword(s, int(strlen(s)));
    while (kwordHashTable[k]) {
        if (++k == KwordHashTableSize)
            k = 0;
    }
    kwordHashTable[k] = number;
}

Tokenizer::Tokenizer(const Location &loc, QFile &in)
{
    init();
    m_in = in.readAll();
    m_pos = 0;
    start(loc);
}

Tokenizer::Tokenizer(const Location &loc, QByteArray in) : m_in(std::move(in))
{
    init();
    m_pos = 0;
    start(loc);
}

Tokenizer::~Tokenizer()
{
    delete[] m_lexBuf1;
    delete[] m_lexBuf2;
}

int Tokenizer::getToken()
{
    token_too_long_warning_was_issued = false;

    char *t = m_prevLex;
    m_prevLex = m_lex;
    m_lex = t;

    while (m_ch != EOF) {
        m_tokLoc = m_curLoc;
        m_lexLen = 0;

        if (isspace(m_ch)) {
            do {
                m_ch = getChar();
            } while (isspace(m_ch));
        } else if (isalpha(m_ch) || m_ch == '_') {
            do {
                m_ch = getChar();
            } while (isalnum(m_ch) || m_ch == '_');

            int k = hashKword(m_lex, int(m_lexLen));
            for (;;) {
                int i = kwordHashTable[k];
                if (i == 0) {
                    return Tok_Ident;
                } else if (i == -1) {
                    if (!m_parsingMacro && ignoredTokensAndDirectives->contains(m_lex)) {
                        if (ignoredTokensAndDirectives->value(m_lex)) { // it's a directive
                            int parenDepth = 0;
                            while (m_ch != EOF && (m_ch != ')' || parenDepth > 1)) {
                                if (m_ch == '(')
                                    ++parenDepth;
                                else if (m_ch == ')')
                                    --parenDepth;
                                m_ch = getChar();
                            }
                            if (m_ch == ')')
                                m_ch = getChar();
                        }
                        break;
                    }
                } else if (strcmp(m_lex, kwords[i - 1]) == 0) {
                    int ret = (int)Tok_FirstKeyword + i - 1;
                    if (ret != Tok_typename)
                        return ret;
                    break;
                }

                if (++k == KwordHashTableSize)
                    k = 0;
            }
        } else if (isdigit(m_ch)) {
            do {
                m_ch = getChar();
            } while (isalnum(m_ch) || m_ch == '.' || m_ch == '+' || m_ch == '-');
            return Tok_Number;
        } else {
            switch (m_ch) {
            case '!':
            case '%':
                m_ch = getChar();
                if (m_ch == '=')
                    m_ch = getChar();
                return Tok_SomeOperator;
            case '"':
                m_ch = getChar();

                while (m_ch != EOF && m_ch != '"') {
                    if (m_ch == '\\')
                        m_ch = getChar();
                    m_ch = getChar();
                }
                m_ch = getChar();

                if (m_ch == EOF)
                    m_tokLoc.warning(
                            QStringLiteral("Unterminated C++ string literal"),
                            QStringLiteral("Maybe you forgot '/*!' at the beginning of the file?"));
                else
                    return Tok_String;
                break;
            case '#':
                return getTokenAfterPreprocessor();
            case '&':
                m_ch = getChar();
                /*
                  Removed check for '&&', only interpret '&=' as an operator.
                  '&&' is also used for an rvalue reference. QTBUG-32675
                 */
                if (m_ch == '=') {
                    m_ch = getChar();
                    return Tok_SomeOperator;
                } else {
                    return Tok_Ampersand;
                }
            case '\'':
                m_ch = getChar();
                /*
                  Allow empty character literal. QTBUG-25775
                 */
                if (m_ch == '\'') {
                    m_ch = getChar();
                    break;
                }
                if (m_ch == '\\')
                    m_ch = getChar();
                do {
                    m_ch = getChar();
                } while (m_ch != EOF && m_ch != '\'');

                if (m_ch == EOF) {
                    m_tokLoc.warning(QStringLiteral("Unterminated C++ character literal"));
                } else {
                    m_ch = getChar();
                    return Tok_Number;
                }
                break;
            case '(':
                m_ch = getChar();
                if (m_numPreprocessorSkipping == 0)
                    m_parenDepth++;
                if (isspace(m_ch)) {
                    do {
                        m_ch = getChar();
                    } while (isspace(m_ch));
                    m_lexLen = 1;
                    m_lex[1] = '\0';
                }
                if (m_ch == '*') {
                    m_ch = getChar();
                    return Tok_LeftParenAster;
                }
                return Tok_LeftParen;
            case ')':
                m_ch = getChar();
                if (m_numPreprocessorSkipping == 0)
                    m_parenDepth--;
                return Tok_RightParen;
            case '*':
                m_ch = getChar();
                if (m_ch == '=') {
                    m_ch = getChar();
                    return Tok_SomeOperator;
                } else {
                    return Tok_Aster;
                }
            case '^':
                m_ch = getChar();
                if (m_ch == '=') {
                    m_ch = getChar();
                    return Tok_SomeOperator;
                } else {
                    return Tok_Caret;
                }
            case '+':
                m_ch = getChar();
                if (m_ch == '+' || m_ch == '=')
                    m_ch = getChar();
                return Tok_SomeOperator;
            case ',':
                m_ch = getChar();
                return Tok_Comma;
            case '-':
                m_ch = getChar();
                if (m_ch == '-' || m_ch == '=') {
                    m_ch = getChar();
                } else if (m_ch == '>') {
                    m_ch = getChar();
                    if (m_ch == '*')
                        m_ch = getChar();
                }
                return Tok_SomeOperator;
            case '.':
                m_ch = getChar();
                if (m_ch == '*') {
                    m_ch = getChar();
                } else if (m_ch == '.') {
                    do {
                        m_ch = getChar();
                    } while (m_ch == '.');
                    return Tok_Ellipsis;
                } else if (isdigit(m_ch)) {
                    do {
                        m_ch = getChar();
                    } while (isalnum(m_ch) || m_ch == '.' || m_ch == '+' || m_ch == '-');
                    return Tok_Number;
                }
                return Tok_SomeOperator;
            case '/':
                m_ch = getChar();
                if (m_ch == '/') {
                    do {
                        m_ch = getChar();
                    } while (m_ch != EOF && m_ch != '\n');
                } else if (m_ch == '*') {
                    bool metDoc = false; // empty doc is no doc
                    bool metSlashAsterBang = false;
                    bool metAster = false;
                    bool metAsterSlash = false;

                    m_ch = getChar();
                    if (m_ch == '!')
                        metSlashAsterBang = true;

                    while (!metAsterSlash) {
                        if (m_ch == EOF) {
                            m_tokLoc.warning(QStringLiteral("Unterminated C++ comment"));
                            break;
                        } else {
                            if (m_ch == '*') {
                                metAster = true;
                            } else if (metAster && m_ch == '/') {
                                metAsterSlash = true;
                            } else {
                                metAster = false;
                                if (isgraph(m_ch))
                                    metDoc = true;
                            }
                        }
                        m_ch = getChar();
                    }
                    if (metSlashAsterBang && metDoc)
                        return Tok_Doc;
                    else if (m_parenDepth > 0)
                        return Tok_Comment;
                } else {
                    if (m_ch == '=')
                        m_ch = getChar();
                    return Tok_SomeOperator;
                }
                break;
            case ':':
                m_ch = getChar();
                if (m_ch == ':') {
                    m_ch = getChar();
                    return Tok_Gulbrandsen;
                } else {
                    return Tok_Colon;
                }
            case ';':
                m_ch = getChar();
                return Tok_Semicolon;
            case '<':
                m_ch = getChar();
                if (m_ch == '<') {
                    m_ch = getChar();
                    if (m_ch == '=')
                        m_ch = getChar();
                    return Tok_SomeOperator;
                } else if (m_ch == '=') {
                    m_ch = getChar();
                    return Tok_SomeOperator;
                } else {
                    return Tok_LeftAngle;
                }
            case '=':
                m_ch = getChar();
                if (m_ch == '=') {
                    m_ch = getChar();
                    return Tok_SomeOperator;
                } else {
                    return Tok_Equal;
                }
            case '>':
                m_ch = getChar();
                if (m_ch == '>') {
                    m_ch = getChar();
                    if (m_ch == '=')
                        m_ch = getChar();
                    return Tok_SomeOperator;
                } else if (m_ch == '=') {
                    m_ch = getChar();
                    return Tok_SomeOperator;
                } else {
                    return Tok_RightAngle;
                }
            case '?':
                m_ch = getChar();
                return Tok_SomeOperator;
            case '[':
                m_ch = getChar();
                if (m_numPreprocessorSkipping == 0)
                    m_bracketDepth++;
                return Tok_LeftBracket;
            case '\\':
                m_ch = getChar();
                m_ch = getChar(); // skip one character
                break;
            case ']':
                m_ch = getChar();
                if (m_numPreprocessorSkipping == 0)
                    m_bracketDepth--;
                return Tok_RightBracket;
            case '{':
                m_ch = getChar();
                if (m_numPreprocessorSkipping == 0)
                    m_braceDepth++;
                return Tok_LeftBrace;
            case '}':
                m_ch = getChar();
                if (m_numPreprocessorSkipping == 0)
                    m_braceDepth--;
                return Tok_RightBrace;
            case '|':
                m_ch = getChar();
                if (m_ch == '|' || m_ch == '=')
                    m_ch = getChar();
                return Tok_SomeOperator;
            case '~':
                m_ch = getChar();
                return Tok_Tilde;
            case '@':
                m_ch = getChar();
                return Tok_At;
            default:
                // ### We should really prevent qdoc from looking at snippet files rather than
                // ### suppress warnings when reading them.
                if (m_numPreprocessorSkipping == 0
                    && !(m_tokLoc.fileName().endsWith(".qdoc")
                         || m_tokLoc.fileName().endsWith(".js"))) {
                    m_tokLoc.warning(QStringLiteral("Hostile character 0x%1 in C++ source")
                                             .arg((uchar)m_ch, 1, 16));
                }
                m_ch = getChar();
            }
        }
    }

    if (m_preprocessorSkipping.size() > 1) {
        m_tokLoc.warning(QStringLiteral("Expected #endif before end of file"));
        // clear it out or we get an infinite loop!
        while (!m_preprocessorSkipping.isEmpty()) {
            popSkipping();
        }
    }

    strcpy(m_lex, "end-of-input");
    m_lexLen = strlen(m_lex);
    return Tok_Eoi;
}

void Tokenizer::initialize()
{
    Config &config = Config::instance();
    QString versionSym = config.get(CONFIG_VERSIONSYM).asString();
    const QLatin1String defaultEncoding("UTF-8");

    QString sourceEncoding = config.get(CONFIG_SOURCEENCODING).asString(defaultEncoding);
    if (!QStringConverter::encodingForName(sourceEncoding.toUtf8().constData())) {
        Location().warning(QStringLiteral("Source encoding '%1' not supported, using '%2' as default.")
                .arg(sourceEncoding, defaultEncoding));
        sourceEncoding = defaultEncoding;
    }
    sourceDecoder = QStringDecoder(sourceEncoding.toUtf8().constData());
    Q_ASSERT(sourceDecoder.isValid());

    comment = new QRegularExpression("/(?:\\*.*\\*/|/.*\n|/[^\n]*$)", QRegularExpression::InvertedGreedinessOption);
    versionX = new QRegularExpression("$cannot possibly match^");
    if (!versionSym.isEmpty())
        versionX->setPattern("^[ \t]*(?:" + QRegularExpression::escape(versionSym)
                             + ")[ \t]+\"([^\"]*)\"[ \t]*$");
    definedX = new QRegularExpression("^defined ?\\(?([A-Z_0-9a-z]+) ?\\)?$");

    QStringList d{config.get(CONFIG_DEFINES).asStringList()};
    d += "qdoc";
    defines = new QRegularExpression(QRegularExpression::anchoredPattern(d.join('|')));
    falsehoods = new QRegularExpression(QRegularExpression::anchoredPattern(
            config.get(CONFIG_FALSEHOODS).asStringList().join('|')));

    /*
      The keyword hash table is always cleared before any words are inserted.
     */
    memset(kwordHashTable, 0, sizeof(kwordHashTable));
    for (int i = 0; i < Tok_LastKeyword - Tok_FirstKeyword + 1; i++)
        insertKwordIntoHash(kwords[i], i + 1);

    ignoredTokensAndDirectives = new QHash<QByteArray, bool>;

    const QStringList tokens{config.get(LANGUAGE_CPP
                                        + Config::dot
                                        + CONFIG_IGNORETOKENS).asStringList()};
    for (const auto &token : tokens) {
        const QByteArray tb = token.toLatin1();
        ignoredTokensAndDirectives->insert(tb, false);
        insertKwordIntoHash(tb.data(), -1);
    }

    const QStringList directives{config.get(LANGUAGE_CPP
                                            + Config::dot
                                            + CONFIG_IGNOREDIRECTIVES).asStringList()};
    for (const auto &directive : directives) {
        const QByteArray db = directive.toLatin1();
        ignoredTokensAndDirectives->insert(db, true);
        insertKwordIntoHash(db.data(), -1);
    }
}

/*!
  The heap allocated variables are freed here. The keyword
  hash table is not cleared here, but it is cleared in the
  initialize() function, before any keywords are inserted.
 */
void Tokenizer::terminate()
{
    delete comment;
    comment = nullptr;
    delete versionX;
    versionX = nullptr;
    delete definedX;
    definedX = nullptr;
    delete defines;
    defines = nullptr;
    delete falsehoods;
    falsehoods = nullptr;
    delete ignoredTokensAndDirectives;
    ignoredTokensAndDirectives = nullptr;
}

void Tokenizer::init()
{
    m_lexBuf1 = new char[(int)yyLexBufSize];
    m_lexBuf2 = new char[(int)yyLexBufSize];
    m_prevLex = m_lexBuf1;
    m_prevLex[0] = '\0';
    m_lex = m_lexBuf2;
    m_lex[0] = '\0';
    m_lexLen = 0;
    m_preprocessorSkipping.push(false);
    m_numPreprocessorSkipping = 0;
    m_braceDepth = 0;
    m_parenDepth = 0;
    m_bracketDepth = 0;
    m_ch = '\0';
    m_parsingMacro = false;
}

void Tokenizer::start(const Location &loc)
{
    m_tokLoc = loc;
    m_curLoc = loc;
    m_curLoc.start();
    strcpy(m_prevLex, "beginning-of-input");
    strcpy(m_lex, "beginning-of-input");
    m_lexLen = strlen(m_lex);
    m_braceDepth = 0;
    m_parenDepth = 0;
    m_bracketDepth = 0;
    m_ch = '\0';
    m_ch = getChar();
}

/*
  Returns the next token, if # was met.  This function interprets the
  preprocessor directive, skips over any #ifdef'd out tokens, and returns the
  token after all of that.
*/
int Tokenizer::getTokenAfterPreprocessor()
{
    m_ch = getChar();
    while (isspace(m_ch) && m_ch != '\n')
        m_ch = getChar();

    /*
      #directive condition
    */
    QString directive;
    QString condition;

    while (isalpha(m_ch)) {
        directive += QChar(m_ch);
        m_ch = getChar();
    }
    if (!directive.isEmpty()) {
        while (m_ch != EOF && m_ch != '\n') {
            if (m_ch == '\\') {
                m_ch = getChar();
                if (m_ch == '\r')
                    m_ch = getChar();
            }
            condition += QChar(m_ch);
            m_ch = getChar();
        }
        condition.remove(*comment);
        condition = condition.simplified();

        /*
          The #if, #ifdef, #ifndef, #elif, #else, and #endif
          directives have an effect on the skipping stack.  For
          instance, if the code processed so far is

              #if 1
              #if 0
              #if 1
              // ...
              #else

          the skipping stack contains, from bottom to top, false true
          true (assuming 0 is false and 1 is true).  If at least one
          entry of the stack is true, the tokens are skipped.

          This mechanism is simple yet hard to understand.
        */
        if (directive[0] == QChar('i')) {
            if (directive == QString("if"))
                pushSkipping(!isTrue(condition));
            else if (directive == QString("ifdef"))
                pushSkipping(!defines->match(condition).hasMatch());
            else if (directive == QString("ifndef"))
                pushSkipping(defines->match(condition).hasMatch());
        } else if (directive[0] == QChar('e')) {
            if (directive == QString("elif")) {
                bool old = popSkipping();
                if (old)
                    pushSkipping(!isTrue(condition));
                else
                    pushSkipping(true);
            } else if (directive == QString("else")) {
                pushSkipping(!popSkipping());
            } else if (directive == QString("endif")) {
                popSkipping();
            }
        } else if (directive == QString("define")) {
            auto match = versionX->match(condition);
            if (match.hasMatch())
                m_version = match.captured(1);
        }
    }

    int tok;
    do {
        /*
          We set yyLex now, and after getToken() this will be
          yyPrevLex. This way, we skip over the preprocessor
          directive.
        */
        qstrcpy(m_lex, m_prevLex);

        /*
          If getToken() meets another #, it will call
          getTokenAfterPreprocessor() once again, which could in turn
          call getToken() again, etc. Unless there are 10,000 or so
          preprocessor directives in a row, this shouldn't overflow
          the stack.
        */
        tok = getToken();
    } while (m_numPreprocessorSkipping > 0 && tok != Tok_Eoi);
    return tok;
}

/*
  Pushes a new skipping value onto the stack.  This corresponds to entering a
  new #if block.
*/
void Tokenizer::pushSkipping(bool skip)
{
    m_preprocessorSkipping.push(skip);
    if (skip)
        m_numPreprocessorSkipping++;
}

/*
  Pops a skipping value from the stack.  This corresponds to reaching a #endif.
*/
bool Tokenizer::popSkipping()
{
    if (m_preprocessorSkipping.isEmpty()) {
        m_tokLoc.warning(QStringLiteral("Unexpected #elif, #else or #endif"));
        return true;
    }

    bool skip = m_preprocessorSkipping.pop();
    if (skip)
        m_numPreprocessorSkipping--;
    return skip;
}

/*
  Returns \c true if the condition evaluates as true, otherwise false.  The
  condition is represented by a string.  Unsophisticated parsing techniques are
  used.  The preprocessing method could be named StriNg-Oriented PreProcessing,
  as SNOBOL stands for StriNg-Oriented symBOlic Language.
*/
bool Tokenizer::isTrue(const QString &condition)
{
    int firstOr = -1;
    int firstAnd = -1;
    int parenDepth = 0;

    /*
      Find the first logical operator at top level, but be careful
      about precedence. Examples:

          X || Y          // the or
          X || Y || Z     // the leftmost or
          X || Y && Z     // the or
          X && Y || Z     // the or
          (X || Y) && Z   // the and
    */
    for (int i = 0; i < condition.size() - 1; i++) {
        QChar ch = condition[i];
        if (ch == QChar('(')) {
            parenDepth++;
        } else if (ch == QChar(')')) {
            parenDepth--;
        } else if (parenDepth == 0) {
            if (condition[i + 1] == ch) {
                if (ch == QChar('|')) {
                    firstOr = i;
                    break;
                } else if (ch == QChar('&')) {
                    if (firstAnd == -1)
                        firstAnd = i;
                }
            }
        }
    }
    if (firstOr != -1)
        return isTrue(condition.left(firstOr)) || isTrue(condition.mid(firstOr + 2));
    if (firstAnd != -1)
        return isTrue(condition.left(firstAnd)) && isTrue(condition.mid(firstAnd + 2));

    QString t = condition.simplified();
    if (t.isEmpty())
        return true;

    if (t[0] == QChar('!'))
        return !isTrue(t.mid(1));
    if (t[0] == QChar('(') && t.endsWith(QChar(')')))
        return isTrue(t.mid(1, t.size() - 2));

    auto match = definedX->match(t);
    if (match.hasMatch())
        return defines->match(match.captured(1)).hasMatch();
    else
        return !falsehoods->match(t).hasMatch();
}

QString Tokenizer::lexeme() const
{
    return sourceDecoder(m_lex);
}

QString Tokenizer::previousLexeme() const
{
    return sourceDecoder(m_prevLex);
}

QT_END_NAMESPACE
