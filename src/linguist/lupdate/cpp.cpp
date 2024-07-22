// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cpp.h"

#include <translator.h>
#include <QtCore/QBitArray>
#include <QtCore/QTextStream>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE


/* qmake ignore Q_OBJECT */

using namespace Qt::StringLiterals;

static const QString CppMagicComment = u"TRANSLATOR"_s;

size_t qHash(const HashString &str)
{
    if (str.m_hash & 0x80000000)
        str.m_hash = qHash(str.m_str) & 0x7fffffff;
    return str.m_hash;
}

size_t qHash(const HashStringList &list)
{
    if (list.m_hash & 0x80000000) {
        uint hash = 0;
        for (const HashString &qs : list.m_list) {
            hash ^= qHash(qs) ^ 0x6ad9f526;
            hash = ((hash << 13) & 0x7fffffff) | (hash >> 18);
        }
        list.m_hash = hash;
    }
    return list.m_hash;
}

static int nextFileId;

class VisitRecorder {
public:
    VisitRecorder()
    {
        m_ba.resize(nextFileId);
    }
    bool tryVisit(int fileId)
    {
        if (m_ba.at(fileId))
            return false;
        m_ba[fileId] = true;
        return true;
    }
private:
    QBitArray m_ba;
};

class CppParser : private CppParserState {

public:
    CppParser(ParseResults *results = 0);
    void setInput(const QString &in);
    void setInput(QTextStream &ts, const QString &fileName);
    void setTranslator(Translator *_tor) { tor = _tor; }
    void parse(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions);
    void parseInternal(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions);
    const ParseResults *recordResults(bool isHeader);
    void deleteResults() { delete results; }

private:
    struct IfdefState {
        IfdefState() {}
        IfdefState(int _bracketDepth, int _braceDepth, int _parenDepth) :
            bracketDepth(_bracketDepth),
            braceDepth(_braceDepth),
            parenDepth(_parenDepth),
            elseLine(-1)
        {}

        CppParserState state;
        int bracketDepth, bracketDepth1st;
        int braceDepth, braceDepth1st;
        int parenDepth, parenDepth1st;
        int elseLine;
    };

    enum TokenType {
        Tok_Eof, Tok_class, Tok_enum, Tok_friend, Tok_namespace, Tok_using, Tok_return,
        Tok_decltype, Tok_Q_OBJECT, Tok_Access, Tok_Cancel,
        Tok_Ident, Tok_String, Tok_RawString, Tok_Arrow, Tok_Colon, Tok_ColonColon,
        Tok_Equals, Tok_LeftBracket, Tok_RightBracket, Tok_AngleBracket, Tok_QuestionMark,
        Tok_LeftBrace, Tok_RightBrace, Tok_LeftParen, Tok_RightParen, Tok_Comma, Tok_Semicolon,
        Tok_Null, Tok_Integer,
        Tok_QuotedInclude, Tok_AngledInclude
    };

    std::ostream &yyMsg(int line = 0);

    int getChar();
    TokenType lookAheadToSemicolonOrLeftBrace();
    TokenType getToken();

    void processComment();

    bool match(TokenType t);
    bool matchString(QString *s);
    bool matchEncoding();
    bool matchStringOrNull(QString *s);
    bool matchExpression();

    void recordMessage(
        int line, const QString &context, const QString &text, const QString &comment,
        const QString &extracomment, const QString &msgid, const TranslatorMessage::ExtraData &extra,
        bool plural);

    void handleTr(QString &prefix, bool plural);
    void handleTranslate(bool plural);
    void handleTrId(bool plural);
    void handleDeclareTrFunctions();

    void processInclude(const QString &file, ConversionData &cd,
                        const QStringList &includeStack, QSet<QString> &inclusions);

    void saveState(CppParserState *state);
    void loadState(const CppParserState &state);

    static QString stringifyNamespace(int start, const NamespaceList &namespaces);
    static QString stringifyNamespace(const NamespaceList &namespaces)
        { return stringifyNamespace(1, namespaces); }
    static QString joinNamespaces(const QString &one, const QString &two);
    typedef bool (CppParser::*VisitNamespaceCallback)(const Namespace *ns, void *context) const;
    bool visitNamespace(const NamespaceList &namespaces, int nsCount,
                        VisitNamespaceCallback callback, void *context,
                        VisitRecorder &vr, const ParseResults *rslt) const;
    bool visitNamespace(const NamespaceList &namespaces, int nsCount,
                        VisitNamespaceCallback callback, void *context) const;
    bool qualifyOneCallbackOwn(const Namespace *ns, void *context) const;
    bool qualifyOneCallbackUsing(const Namespace *ns, void *context) const;
    bool qualifyOne(const NamespaceList &namespaces, int nsCnt, const HashString &segment,
                    NamespaceList *resolved, QSet<HashStringList> *visitedUsings) const;
    bool qualifyOne(const NamespaceList &namespaces, int nsCnt, const HashString &segment,
                    NamespaceList *resolved) const;
    bool fullyQualify(const NamespaceList &namespaces, int nsCnt,
                      const NamespaceList &segments, bool isDeclaration,
                      NamespaceList *resolved, NamespaceList *unresolved) const;
    bool fullyQualify(const NamespaceList &namespaces,
                      const NamespaceList &segments, bool isDeclaration,
                      NamespaceList *resolved, NamespaceList *unresolved) const;
    bool fullyQualify(const NamespaceList &namespaces,
                      const QString &segments, bool isDeclaration,
                      NamespaceList *resolved, NamespaceList *unresolved) const;
    bool findNamespaceCallback(const Namespace *ns, void *context) const;
    const Namespace *findNamespace(const NamespaceList &namespaces, int nsCount = -1) const;
    void enterNamespace(NamespaceList *namespaces, const HashString &name);
    void truncateNamespaces(NamespaceList *namespaces, int lenght);
    Namespace *modifyNamespace(NamespaceList *namespaces, bool haveLast = true);

    // Tokenizer state
    QString yyFileName;
    int yyCh;
    bool yyAtNewline;
    QString yyWord;
    qsizetype yyWordInitialCapacity = 0;
    QStack<IfdefState> yyIfdefStack;
    int yyBracketDepth;
    int yyBraceDepth;
    int yyParenDepth;
    int yyLineNo;
    int yyCurLineNo;
    int yyBracketLineNo;
    int yyBraceLineNo;
    int yyParenLineNo;

    // the string to read from and current position in the string
    QStringConverter::Encoding yySourceEncoding = QStringConverter::Utf8;
    QString yyInStr;
    const ushort *yyInPtr;

    // Parser state
    TokenType yyTok;

    bool metaExpected;
    QString context;
    QString text;
    QString comment;
    QString extracomment;
    QString msgid;
    QString sourcetext;
    TranslatorMessage::ExtraData extra;

    QString prospectiveContext;
    ParseResults *results;
    Translator *tor;
    bool directInclude;

    CppParserState savedState;
    int yyMinBraceDepth;
    bool inDefine;
};

CppParser::CppParser(ParseResults *_results)
{
    tor = 0;
    if (_results) {
        results = _results;
        directInclude = true;
    } else {
        results = new ParseResults;
        directInclude = false;
    }
    yyBracketDepth = 0;
    yyBraceDepth = 0;
    yyParenDepth = 0;
    yyCurLineNo = 1;
    yyBracketLineNo = 1;
    yyBraceLineNo = 1;
    yyParenLineNo = 1;
    yyAtNewline = true;
    yyMinBraceDepth = 0;
    inDefine = false;
}


std::ostream &CppParser::yyMsg(int line)
{
    return std::cerr << qPrintable(yyFileName) << ':' << (line ? line : yyLineNo) << ": ";
}

void CppParser::setInput(const QString &in)
{
    yyInStr = in;
    yyFileName = QString();
    yySourceEncoding = QStringConverter::Utf8;
}

void CppParser::setInput(QTextStream &ts, const QString &fileName)
{
    yyInStr = ts.readAll();
    yyFileName = fileName;
    yySourceEncoding = ts.encoding();
}

/*
  The first part of this source file is the C++ tokenizer.  We skip
  most of C++; the only tokens that interest us are defined here.
  Thus, the code fragment

      int main()
      {
          printf("Hello, world!\n");
          return 0;
      }

  is broken down into the following tokens (Tok_ omitted):

      Ident Ident LeftParen RightParen
      LeftBrace
          Ident LeftParen String RightParen Semicolon
          return Semicolon
      RightBrace.

  The 0 doesn't produce any token.
*/

int CppParser::getChar()
{
    const ushort *uc = yyInPtr;
    forever {
        ushort c = *uc;
        if (!c) {
            yyInPtr = uc;
            return EOF;
        }
        ++uc;
        if (c == '\\') {
            ushort cc = *uc;
            if (cc == '\n') {
                ++yyCurLineNo;
                ++uc;
                continue;
            }
            if (cc == '\r') {
                ++yyCurLineNo;
                ++uc;
                if (*uc == '\n')
                    ++uc;
                continue;
            }
        }
        if (c == '\r') {
            if (*uc == '\n')
                ++uc;
            c = '\n';
            ++yyCurLineNo;
            yyAtNewline = true;
        } else if (c == '\n') {
            ++yyCurLineNo;
            yyAtNewline = true;
        } else if (c != ' ' && c != '\t' && c != '#') {
            yyAtNewline = false;
        }
        yyInPtr = uc;
        return int(c);
    }
}

CppParser::TokenType CppParser::lookAheadToSemicolonOrLeftBrace()
{
    if (*yyInPtr == 0)
        return Tok_Eof;
    const ushort *uc = yyInPtr + 1;
    forever {
        ushort c = *uc;
        if (!c)
            return Tok_Eof;
        if (c == ';')
            return Tok_Semicolon;
        if (c == '{')
            return Tok_LeftBrace;
        ++uc;
    }
}

static bool isStringLiteralPrefix(const QStringView s)
{
    return s == u"L"_s
        || s == u"U"_s
        || s == u"u"_s
        || s == u"u8"_s;
}

static bool isRawStringLiteralPrefix(QStringView s)
{
    if (s.endsWith(u'R')) {
        s.chop(1);
        return s.isEmpty() || isStringLiteralPrefix(s);
    }
    return false;
}

static const QString strQ_OBJECT = u"Q_OBJECT"_s;
static const QString strclass = u"class"_s;
static const QString strdecltype = u"decltype"_s;
static const QString strenum = u"enum"_s;
static const QString strfinal = u"final"_s;
static const QString strfriend = u"friend"_s;
static const QString strnamespace = u"namespace"_s;
static const QString strnullptr = u"nullptr"_s;
static const QString strQ_NULLPTR = u"Q_NULLPTR"_s;
static const QString strNULL = u"NULL"_s;
static const QString stroperator = u"operator"_s;
static const QString strreturn = u"return"_s;
static const QString strstruct = u"struct"_s;
static const QString strusing = u"using"_s;
static const QString strprivate = u"private"_s;
static const QString strprotected = u"protected"_s;
static const QString strpublic = u"public"_s;
static const QString strslots = u"slots"_s;
static const QString strsignals = u"signals"_s;
static const QString strQ_SLOTS = u"Q_SLOTS"_s;
static const QString strQ_SIGNALS = u"Q_SIGNALS"_s;

CppParser::TokenType CppParser::getToken()
{
  restart:
    // Failing this assertion would mean losing the preallocated buffer.
    Q_ASSERT(yyWord.capacity() == yyWordInitialCapacity);

    while (yyCh != EOF) {
        yyLineNo = yyCurLineNo;

        if (yyCh == '#' && yyAtNewline) {
            /*
              Early versions of lupdate complained about
              unbalanced braces in the following code:

                  #ifdef ALPHA
                      while (beta) {
                  #else
                      while (gamma) {
                  #endif
                          delta;
                      }

              The code contains, indeed, two opening braces for
              one closing brace; yet there's no reason to panic.

              The solution is to remember yyBraceDepth as it was
              when #if, #ifdef or #ifndef was met, and to set
              yyBraceDepth to that value when meeting #elif or
              #else.
            */
            do {
                yyCh = getChar();
            } while (isspace(yyCh) && yyCh != '\n');

            switch (yyCh) {
            case 'd': // define
                // Skip over the name of the define to avoid it being interpreted as c++ code
                do { // Rest of "define"
                    yyCh = getChar();
                    if (yyCh == EOF)
                        return Tok_Eof;
                    if (yyCh == '\n')
                        goto restart;
                } while (!isspace(yyCh));
                do { // Space beween "define" and macro name
                    yyCh = getChar();
                    if (yyCh == EOF)
                        return Tok_Eof;
                    if (yyCh == '\n')
                        goto restart;
                } while (isspace(yyCh));
                do { // Macro name
                    if (yyCh == '(') {
                        // Argument list. Follows the name without a space, and no
                        // paren nesting is possible.
                        do {
                            yyCh = getChar();
                            if (yyCh == EOF)
                                return Tok_Eof;
                            if (yyCh == '\n')
                                goto restart;
                        } while (yyCh != ')');
                        break;
                    }
                    yyCh = getChar();
                    if (yyCh == EOF)
                        return Tok_Eof;
                    if (yyCh == '\n')
                        goto restart;
                } while (!isspace(yyCh));
                do { // Shortcut the immediate newline case if no comments follow.
                    yyCh = getChar();
                    if (yyCh == EOF)
                        return Tok_Eof;
                    if (yyCh == '\n')
                        goto restart;
                } while (isspace(yyCh));

                saveState(&savedState);
                yyMinBraceDepth = yyBraceDepth;
                inDefine = true;
                goto restart;
            case 'i':
                yyCh = getChar();
                if (yyCh == 'f') {
                    // if, ifdef, ifndef
                    yyIfdefStack.push(IfdefState(yyBracketDepth, yyBraceDepth, yyParenDepth));
                    yyCh = getChar();
                } else if (yyCh == 'n') {
                    // include
                    do {
                        yyCh = getChar();
                    } while (yyCh != EOF && !isspace(yyCh) && yyCh != '"' && yyCh != '<' );
                    while (isspace(yyCh))
                        yyCh = getChar();
                    int tChar;
                    if (yyCh == '"')
                        tChar = '"';
                    else if (yyCh == '<')
                        tChar = '>';
                    else
                        break;
                    ushort *ptr = (ushort *)yyWord.unicode();
                    forever {
                        yyCh = getChar();
                        if (yyCh == EOF || yyCh == '\n')
                            break;
                        if (yyCh == tChar) {
                            yyCh = getChar();
                            break;
                        }
                        *ptr++ = yyCh;
                    }
                    yyWord.resize(ptr - (ushort *)yyWord.unicode());
                    return (tChar == '"') ? Tok_QuotedInclude : Tok_AngledInclude;
                }
                break;
            case 'e':
                yyCh = getChar();
                if (yyCh == 'l') {
                    // elif, else
                    if (!yyIfdefStack.isEmpty()) {
                        IfdefState &is = yyIfdefStack.top();
                        if (is.elseLine != -1) {
                            if (yyBracketDepth != is.bracketDepth1st
                                || yyBraceDepth != is.braceDepth1st
                                || yyParenDepth != is.parenDepth1st)
                                yyMsg(is.elseLine)
                                    << "Parenthesis/bracket/brace mismatch between "
                                       "#if and #else branches; using #if branch\n";
                        } else {
                            is.bracketDepth1st = yyBracketDepth;
                            is.braceDepth1st = yyBraceDepth;
                            is.parenDepth1st = yyParenDepth;
                            saveState(&is.state);
                        }
                        is.elseLine = yyLineNo;
                        yyBracketDepth = is.bracketDepth;
                        yyBraceDepth = is.braceDepth;
                        yyParenDepth = is.parenDepth;
                    }
                    yyCh = getChar();
                } else if (yyCh == 'n') {
                    // endif
                    if (!yyIfdefStack.isEmpty()) {
                        IfdefState is = yyIfdefStack.pop();
                        if (is.elseLine != -1) {
                            if (yyBracketDepth != is.bracketDepth1st
                                || yyBraceDepth != is.braceDepth1st
                                || yyParenDepth != is.parenDepth1st)
                                yyMsg(is.elseLine)
                                    << "Parenthesis/brace mismatch between "
                                       "#if and #else branches; using #if branch\n";
                            yyBracketDepth = is.bracketDepth1st;
                            yyBraceDepth = is.braceDepth1st;
                            yyParenDepth = is.parenDepth1st;
                            loadState(is.state);
                        }
                    }
                    yyCh = getChar();
                }
                break;
            }
            // Optimization: skip over rest of preprocessor directive
            do {
                if (yyCh == '/') {
                    yyCh = getChar();
                    if (yyCh == '/') {
                        do {
                            yyCh = getChar();
                        } while (yyCh != EOF && yyCh != '\n');
                        break;
                    } else if (yyCh == '*') {
                        bool metAster = false;

                        forever {
                            yyCh = getChar();
                            if (yyCh == EOF) {
                                yyMsg() << "Unterminated C++ comment\n";
                                break;
                            }

                            if (yyCh == '*') {
                                metAster = true;
                            } else if (metAster && yyCh == '/') {
                                yyCh = getChar();
                                break;
                            } else {
                                metAster = false;
                            }
                        }
                    }
                } else {
                    yyCh = getChar();
                }
            } while (yyCh != '\n' && yyCh != EOF);
            yyCh = getChar();
        } else if ((yyCh >= 'A' && yyCh <= 'Z') || (yyCh >= 'a' && yyCh <= 'z') || yyCh == '_') {
            ushort *ptr = (ushort *)yyWord.unicode();
            do {
                *ptr++ = yyCh;
                yyCh = getChar();
            } while ((yyCh >= 'A' && yyCh <= 'Z') || (yyCh >= 'a' && yyCh <= 'z')
                     || (yyCh >= '0' && yyCh <= '9') || yyCh == '_');
            yyWord.resize(ptr - (ushort *)yyWord.unicode());

            //qDebug() << "IDENT: " << yyWord;

            if (yyCh == '"' && isStringLiteralPrefix(yyWord)) {
                // Handle prefixed string literals as ordinary string literals.
                continue;
            }

            switch (yyWord.unicode()[0].unicode()) {
            case 'N':
                if (yyWord == strNULL)
                    return Tok_Null;
                break;
            case 'Q':
                if (yyWord == strQ_NULLPTR)
                    return Tok_Null;
                if (yyWord == strQ_OBJECT)
                    return Tok_Q_OBJECT;
                if (yyWord == strQ_SLOTS || yyWord == strQ_SIGNALS)
                    return Tok_Access;
                break;
            case 'c':
                if (yyWord == strclass)
                    return Tok_class;
                break;
            case 'd':
                if (yyWord == strdecltype)
                    return Tok_decltype;
                break;
            case 'e':
                if (yyWord == strenum)
                    return Tok_enum;
                break;
            case 'f':
                if (yyWord == strfriend)
                    return Tok_friend;
                break;
            case 'n':
                if (yyWord == strnamespace)
                    return Tok_namespace;
                if (yyWord == strnullptr)
                    return Tok_Null;
                break;
            case 'o':
                if (yyWord == stroperator) {
                    // Operator overload declaration/definition.
                    // We need to prevent those characters from confusing the followup
                    // parsing. Actually using them does not add value, so just eat them.
                    while (isspace(yyCh))
                       yyCh = getChar();
                    while (yyCh == '+' || yyCh == '-' || yyCh == '*' || yyCh == '/' || yyCh == '%'
                           || yyCh == '=' || yyCh == '<' || yyCh == '>' || yyCh == '!'
                           || yyCh == '&' || yyCh == '|' || yyCh == '~' || yyCh == '^'
                           || yyCh == '[' || yyCh == ']')
                        yyCh = getChar();
                }
                break;
            case 'p':
                if (yyWord == strpublic || yyWord == strprotected || yyWord == strprivate)
                    return Tok_Access;
                break;
            case 'r':
                if (yyWord == strreturn)
                    return Tok_return;
                break;
            case 's':
                if (yyWord == strstruct)
                    return Tok_class;
                if (yyWord == strslots || yyWord == strsignals)
                    return Tok_Access;
                break;
            case 'u':
                if (yyWord == strusing)
                    return Tok_using;
                break;
            }

            // a C++11 raw string literal?
            if (yyCh == '"' && isRawStringLiteralPrefix(yyWord)) {
                ptr = reinterpret_cast<ushort *>(const_cast<QChar *>(yyWord.unicode()));
                //get delimiter
                QString delimiter;
                for (yyCh = getChar(); yyCh != EOF && yyCh != '('; yyCh = getChar())
                    delimiter += QLatin1Char(yyCh);
                if (yyCh != EOF)
                    yyCh = getChar(); // throw away the opening parentheses
                bool is_end = false;
                ushort *ptr_past_end = nullptr;
                while (yyCh != EOF && !is_end) {
                    *ptr++ = yyCh;
                    if (ptr_past_end != nullptr) {
                        if (delimiter.size() == ptr - ptr_past_end
                                && memcmp(delimiter.unicode(), ptr_past_end, (ptr - ptr_past_end) * sizeof (ushort)) == 0
                           ) {
                            // we've got the delimiter, check if " follows
                            yyCh = getChar();
                            if (yyCh == '"')
                                is_end = true;
                            else
                                ptr_past_end = nullptr;
                            continue;
                        }
                    }
                    if (yyCh == ')') {
                        ptr_past_end = ptr;
                        if (delimiter.isEmpty()) {
                            // no delimiter, check if " follows
                            yyCh = getChar();
                            if (yyCh == '"')
                                is_end = true;
                            else
                                ptr_past_end = nullptr;
                            continue;
                        }
                    }
                    yyCh = getChar();
                }
                if (is_end)
                    yyWord.resize(ptr_past_end - 1 - reinterpret_cast<const ushort *>(yyWord.unicode()));
                else
                    yyWord.resize(ptr - reinterpret_cast<const ushort *>(yyWord.unicode()));
                if (yyCh != '"')
                    yyMsg() << "Unterminated/mismatched C++ Raw string\n";
                else
                    yyCh = getChar();
                return Tok_RawString;
            }

            return Tok_Ident;
        } else {
            switch (yyCh) {
            case '\n':
                if (inDefine) {
                    loadState(savedState);
                    prospectiveContext.clear();
                    yyBraceDepth = yyMinBraceDepth;
                    yyMinBraceDepth = 0;
                    inDefine = false;
                    metaExpected = true;
                    yyCh = getChar();
                    return Tok_Cancel; // Break out of any multi-token constructs
                }
                yyCh = getChar();
                break;
            case '/':
                yyCh = getChar();
                if (yyCh == '/') {
                    ushort *ptr = (ushort *)yyWord.unicode();
                    do {
                        yyCh = getChar();
                        if (yyCh == EOF)
                            break;
                        *ptr++ = yyCh;
                    } while (yyCh != '\n');
                    yyWord.resize(ptr - (ushort *)yyWord.unicode());
                    processComment();
                } else if (yyCh == '*') {
                    bool metAster = false;
                    ushort *ptr = (ushort *)yyWord.unicode();

                    forever {
                        yyCh = getChar();
                        if (yyCh == EOF) {
                            yyMsg() << "Unterminated C++ comment\n";
                            break;
                        }
                        *ptr++ = yyCh;

                        if (yyCh == '*')
                            metAster = true;
                        else if (metAster && yyCh == '/')
                            break;
                        else
                            metAster = false;
                    }
                    yyWord.resize(ptr - (ushort *)yyWord.unicode() - 2);
                    processComment();

                    yyCh = getChar();
                }
                break;
            case '"': {
                ushort *ptr = (ushort *)yyWord.unicode();
                yyCh = getChar();
                while (yyCh != EOF && yyCh != '\n' && yyCh != '"') {
                    if (yyCh == '\\') {
                        yyCh = getChar();
                        if (yyCh == EOF || yyCh == '\n')
                            break;
                        *ptr++ = '\\';
                    }
                    *ptr++ = yyCh;
                    yyCh = getChar();
                }
                yyWord.resize(ptr - (ushort *)yyWord.unicode());

                if (yyCh != '"')
                    yyMsg() << "Unterminated C++ string\n";
                else
                    yyCh = getChar();
                return Tok_String;
            }
            case '-':
                yyCh = getChar();
                if (yyCh == '>') {
                    yyCh = getChar();
                    return Tok_Arrow;
                }
                break;
            case ':':
                yyCh = getChar();
                if (yyCh == ':') {
                    yyCh = getChar();
                    return Tok_ColonColon;
                }
                return Tok_Colon;
            // Incomplete: '<' might be part of '<=' or of template syntax.
            // The main intent of not completely ignoring it is to break
            // parsing of things like   std::cout << QObject::tr()  as
            // context std::cout::QObject (see Task 161106)
            case '=':
                yyCh = getChar();
                return Tok_Equals;
            case '>':
            case '<':
                yyCh = getChar();
                return Tok_AngleBracket;
            case '\'':
                yyCh = getChar();
                if (yyCh == '\\')
                    yyCh = getChar();

                forever {
                    if (yyCh == EOF || yyCh == '\n') {
                        yyMsg() << "Unterminated C++ character\n";
                        break;
                    }
                    yyCh = getChar();
                    if (yyCh == '\'') {
                        yyCh = getChar();
                        break;
                    }
                }
                break;
            case '{':
                if (yyBraceDepth == 0)
                    yyBraceLineNo = yyCurLineNo;
                yyBraceDepth++;
                yyCh = getChar();
                return Tok_LeftBrace;
            case '}':
                if (yyBraceDepth == yyMinBraceDepth) {
                    if (!inDefine)
                        yyMsg(yyCurLineNo)
                            << "Excess closing brace in C++ code"
                            " (or abuse of the C++ preprocessor)\n";
                    // Avoid things getting messed up even more
                    yyCh = getChar();
                    return Tok_Semicolon;
                }
                yyBraceDepth--;
                yyCh = getChar();
                return Tok_RightBrace;
            case '(':
                if (yyParenDepth == 0)
                    yyParenLineNo = yyCurLineNo;
                yyParenDepth++;
                yyCh = getChar();
                return Tok_LeftParen;
            case ')':
                if (yyParenDepth == 0)
                    yyMsg(yyCurLineNo)
                        << "Excess closing parenthesis in C++ code"
                           " (or abuse of the C++ preprocessor)\n";
                else
                    yyParenDepth--;
                yyCh = getChar();
                return Tok_RightParen;
            case '[':
                if (yyBracketDepth == 0)
                    yyBracketLineNo = yyCurLineNo;
                yyBracketDepth++;
                yyCh = getChar();
                return Tok_LeftBracket;
            case ']':
                if (yyBracketDepth == 0)
                    yyMsg(yyCurLineNo)
                        << "Excess closing bracket in C++ code"
                           " (or abuse of the C++ preprocessor)\n";
                else
                    yyBracketDepth--;
                yyCh = getChar();
                return Tok_RightBracket;
            case ',':
                yyCh = getChar();
                return Tok_Comma;
            case ';':
                yyCh = getChar();
                return Tok_Semicolon;
            case '?':
                yyCh = getChar();
                return Tok_QuestionMark;
            case '0':
                yyCh = getChar();
                if (yyCh == 'x' || yyCh == 'X') {
                    do {
                        yyCh = getChar();
                    } while ((yyCh >= '0' && yyCh <= '9') || yyCh == '\''
                             || (yyCh >= 'a' && yyCh <= 'f') || (yyCh >= 'A' && yyCh <= 'F'));
                    return Tok_Integer;
                }
                if (yyCh < '0' || yyCh > '9')
                    return Tok_Null;
                Q_FALLTHROUGH();
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                do {
                    yyCh = getChar();
                } while ((yyCh >= '0' && yyCh <= '9') || yyCh == '\'');
                return Tok_Integer;
            default:
                yyCh = getChar();
                break;
            }
        }
    }
    return Tok_Eof;
}

/*
  The second part of this source file are namespace/class related
  utilities for the third part.
*/

void CppParser::saveState(CppParserState *state)
{
    *state = *this;
}

void CppParser::loadState(const CppParserState &state)
{
    *static_cast<CppParserState *>(this) = state;
}

Namespace *CppParser::modifyNamespace(NamespaceList *namespaces, bool haveLast)
{
    Namespace *pns, *ns = &results->rootNamespace;
    for (int i = 1; i < namespaces->size(); ++i) {
        pns = ns;
        if (!(ns = pns->children.value(namespaces->at(i)))) {
            do {
                ns = new Namespace;
                if (haveLast || i < namespaces->size() - 1)
                    if (const Namespace *ons = findNamespace(*namespaces, i + 1))
                        ns->classDef = ons->classDef;
                pns->children.insert(namespaces->at(i), ns);
                pns = ns;
            } while (++i < namespaces->size());
            break;
        }
    }
    return ns;
}

QString CppParser::stringifyNamespace(int start, const NamespaceList &namespaces)
{
    QString ret;
    int l = 0;
    for (int j = start; j < namespaces.size(); ++j)
        l += namespaces.at(j).value().size();
    ret.reserve(l + qMax(0, (namespaces.size() - start - 1)) * 2);
    for (int i = start; i < namespaces.size(); ++i) {
        if (i > start)
            ret += QLatin1String("::");
        ret += namespaces.at(i).value();
    }
    return ret;
}

QString CppParser::joinNamespaces(const QString &one, const QString &two)
{
    return two.isEmpty() ? one : one.isEmpty() ? two : one + QStringLiteral("::") + two;
}

bool CppParser::visitNamespace(const NamespaceList &namespaces, int nsCount,
                               VisitNamespaceCallback callback, void *context,
                               VisitRecorder &vr, const ParseResults *rslt) const
{
    const Namespace *ns = &rslt->rootNamespace;
    for (int i = 1; i < nsCount; ++i)
        if (!(ns = ns->children.value(namespaces.at(i))))
            goto supers;
    if ((this->*callback)(ns, context))
        return true;
supers:
    for (const ParseResults *sup : rslt->includes)
        if (vr.tryVisit(sup->fileId)
            && visitNamespace(namespaces, nsCount, callback, context, vr, sup))
            return true;
    return false;
}

bool CppParser::visitNamespace(const NamespaceList &namespaces, int nsCount,
                               VisitNamespaceCallback callback, void *context) const
{
    VisitRecorder vr;
    return visitNamespace(namespaces, nsCount, callback, context, vr, results);
}

struct QualifyOneData {
    QualifyOneData(const NamespaceList &ns, int nsc, const HashString &seg, NamespaceList *rslvd,
                   QSet<HashStringList> *visited)
        : namespaces(ns), nsCount(nsc), segment(seg), resolved(rslvd), visitedUsings(visited)
    {}

    const NamespaceList &namespaces;
    int nsCount;
    const HashString &segment;
    NamespaceList *resolved;
    QSet<HashStringList> *visitedUsings;
};

bool CppParser::qualifyOneCallbackOwn(const Namespace *ns, void *context) const
{
    QualifyOneData *data = (QualifyOneData *)context;
    if (ns->children.contains(data->segment)) {
        *data->resolved = data->namespaces.mid(0, data->nsCount);
        *data->resolved << data->segment;
        return true;
    }
    auto nsai = ns->aliases.constFind(data->segment);
    if (nsai != ns->aliases.constEnd()) {
        const NamespaceList &nsl = *nsai;
        if (nsl.last().value().isEmpty()) { // Delayed alias resolution
            NamespaceList &nslIn = *const_cast<NamespaceList *>(&nsl);
            nslIn.removeLast();
            NamespaceList nslOut;
            if (!fullyQualify(data->namespaces, data->nsCount, nslIn, false, &nslOut, 0)) {
                const_cast<Namespace *>(ns)->aliases.remove(data->segment);
                return false;
            }
            nslIn = nslOut;
        }
        *data->resolved = nsl;
        return true;
    }
    return false;
}

bool CppParser::qualifyOneCallbackUsing(const Namespace *ns, void *context) const
{
    QualifyOneData *data = (QualifyOneData *)context;
    for (const HashStringList &use : ns->usings)
        if (!data->visitedUsings->contains(use)) {
            data->visitedUsings->insert(use);
            if (qualifyOne(use.value(), use.value().size(), data->segment, data->resolved,
                           data->visitedUsings))
                return true;
        }
    return false;
}

bool CppParser::qualifyOne(const NamespaceList &namespaces, int nsCnt, const HashString &segment,
                           NamespaceList *resolved, QSet<HashStringList> *visitedUsings) const
{
    QualifyOneData data(namespaces, nsCnt, segment, resolved, visitedUsings);

    if (visitNamespace(namespaces, nsCnt, &CppParser::qualifyOneCallbackOwn, &data))
        return true;

    return visitNamespace(namespaces, nsCnt, &CppParser::qualifyOneCallbackUsing, &data);
}

bool CppParser::qualifyOne(const NamespaceList &namespaces, int nsCnt, const HashString &segment,
                           NamespaceList *resolved) const
{
    QSet<HashStringList> visitedUsings;

    return qualifyOne(namespaces, nsCnt, segment, resolved, &visitedUsings);
}

bool CppParser::fullyQualify(const NamespaceList &namespaces, int nsCnt,
                             const NamespaceList &segments, bool isDeclaration,
                             NamespaceList *resolved, NamespaceList *unresolved) const
{
    int nsIdx;
    int initSegIdx;

    if (segments.first().value().isEmpty()) {
        // fully qualified
        if (segments.size() == 1) {
            resolved->clear();
            *resolved << HashString(QString());
            return true;
        }
        initSegIdx = 1;
        nsIdx = 0;
    } else {
        initSegIdx = 0;
        nsIdx = nsCnt - 1;
    }

    do {
        if (qualifyOne(namespaces, nsIdx + 1, segments[initSegIdx], resolved)) {
            int segIdx = initSegIdx;
            while (++segIdx < segments.size()) {
                if (!qualifyOne(*resolved, resolved->size(), segments[segIdx], resolved)) {
                    if (unresolved)
                        *unresolved = segments.mid(segIdx);
                    return false;
                }
            }
            return true;
        }
    } while (!isDeclaration && --nsIdx >= 0);
    resolved->clear();
    *resolved << HashString(QString());
    if (unresolved)
        *unresolved = segments.mid(initSegIdx);
    return false;
}

bool CppParser::fullyQualify(const NamespaceList &namespaces,
                             const NamespaceList &segments, bool isDeclaration,
                             NamespaceList *resolved, NamespaceList *unresolved) const
{
    return fullyQualify(namespaces, namespaces.size(),
                        segments, isDeclaration, resolved, unresolved);
}

bool CppParser::fullyQualify(const NamespaceList &namespaces,
                             const QString &quali, bool isDeclaration,
                             NamespaceList *resolved, NamespaceList *unresolved) const
{
    static QString strColons(QLatin1String("::"));

    NamespaceList segments;
    for (const QString &str : quali.split(strColons)) // XXX slow, but needs to be fast(?)
        segments << HashString(str);
    return fullyQualify(namespaces, segments, isDeclaration, resolved, unresolved);
}

bool CppParser::findNamespaceCallback(const Namespace *ns, void *context) const
{
    *((const Namespace **)context) = ns;
    return true;
}

const Namespace *CppParser::findNamespace(const NamespaceList &namespaces, int nsCount) const
{
    const Namespace *ns = 0;
    if (nsCount == -1)
        nsCount = namespaces.size();
    visitNamespace(namespaces, nsCount, &CppParser::findNamespaceCallback, &ns);
    return ns;
}

void CppParser::enterNamespace(NamespaceList *namespaces, const HashString &name)
{
    *namespaces << name;
    if (!findNamespace(*namespaces))
        modifyNamespace(namespaces, false);
}

void CppParser::truncateNamespaces(NamespaceList *namespaces, int length)
{
    if (namespaces->size() > length)
        namespaces->erase(namespaces->begin() + length, namespaces->end());
}


/*
  Functions for processing include files.
*/

size_t qHash(const CppParserState &s, size_t seed)
{
    seed = qHash(s.namespaces, seed);
    seed = qHash(s.namespaceDepths, seed);
    seed = qHash(s.functionContext, seed);
    seed = qHash(s.functionContextUnresolved, seed);
    seed = qHash(s.pendingContext, seed);
    return seed;
}

size_t qHash(const ResultsCacheKey &key, size_t seed)
{
    seed = qHash(key.cleanFile, seed);
    seed = qHash(key.parserState, seed);
    return seed;
}

IncludeCycleHash &CppFiles::includeCycles()
{
    static IncludeCycleHash cycles;

    return cycles;
}

TranslatorHash &CppFiles::translatedFiles()
{
    static TranslatorHash tors;

    return tors;
}

QSet<QString> &CppFiles::blacklistedFiles()
{
    static QSet<QString> blacklisted;

    return blacklisted;
}

QSet<const ParseResults *> CppFiles::getResults(const ResultsCacheKey &key)
{
    IncludeCycle * const cycle = includeCycles().value(key);

    if (cycle)
        return cycle->results;
    else
        return QSet<const ParseResults *>();
}

void CppFiles::setResults(const ResultsCacheKey &key, const ParseResults *results)
{
    IncludeCycle *cycle = includeCycles().value(key);

    if (!cycle) {
        cycle = new IncludeCycle;
        includeCycles().insert(key, cycle);
    }

    cycle->fileNames.insert(key.cleanFile);
    cycle->results.insert(results);
}

const Translator *CppFiles::getTranslator(const QString &cleanFile)
{
    return translatedFiles().value(cleanFile);
}

void CppFiles::setTranslator(const QString &cleanFile, const Translator *tor)
{
    translatedFiles().insert(cleanFile, tor);
}

bool CppFiles::isBlacklisted(const QString &cleanFile)
{
    return blacklistedFiles().contains(cleanFile);
}

void CppFiles::setBlacklisted(const QString &cleanFile)
{
    blacklistedFiles().insert(cleanFile);
}

void CppFiles::addIncludeCycle(const QSet<QString> &fileNames, const CppParserState &parserState)
{
    IncludeCycle * const cycle = new IncludeCycle;
    cycle->fileNames = fileNames;

    QSet<IncludeCycle *> intersectingCycles;
    for (const QString &fileName : fileNames) {
        const ResultsCacheKey key = { fileName, parserState };
        IncludeCycle *intersectingCycle = includeCycles().value(key);

        if (intersectingCycle && !intersectingCycles.contains(intersectingCycle)) {
            intersectingCycles.insert(intersectingCycle);

            cycle->fileNames.unite(intersectingCycle->fileNames);
            cycle->results.unite(intersectingCycle->results);
        }
    }
    qDeleteAll(intersectingCycles);

    for (const QString &fileName : std::as_const(cycle->fileNames))
        includeCycles().insert({ fileName, parserState }, cycle);
}

static bool isHeader(const QString &name)
{
    QString fileExt = QFileInfo(name).suffix();
    return fileExt.isEmpty() || fileExt.startsWith(QLatin1Char('h'), Qt::CaseInsensitive);
}

void CppParser::processInclude(const QString &file, ConversionData &cd, const QStringList &includeStack,
                               QSet<QString> &inclusions)
{
    QString cleanFile = QDir::cleanPath(file);

    for (const QRegularExpression &rx : std::as_const(cd.m_excludes)) {
        if (rx.match(cleanFile).hasMatch())
            return;
    }

    const int index = includeStack.indexOf(cleanFile);
    if (index != -1) {
        CppFiles::addIncludeCycle(QSet<QString>(includeStack.cbegin() + index, includeStack.cend()),
                                  *this);
        return;
    }

    // If the #include has been blacklisted previously,
    // or is not a header file (stdc++ extensionless or *.h*), then really include
    // it. Otherwise it is safe to process it stand-alone and re-use the parsed
    // namespace data for inclusion into other files.
    bool isIndirect = false;
    if (!CppFiles::isBlacklisted(cleanFile)
        && isHeader(cleanFile)) {

        QSet<const ParseResults *> res = CppFiles::getResults(ResultsCacheKey(cleanFile, *this));
        if (!res.isEmpty()) {
            results->includes.unite(res);
            return;
        }

        isIndirect = true;
    }

    QFile f(cleanFile);
    if (!f.open(QIODevice::ReadOnly)) {
        yyMsg() << qPrintable(
            QStringLiteral("Cannot open %1: %2\n").arg(cleanFile, f.errorString()));
        return;
    }

    QTextStream ts(&f);
    ts.setEncoding(yySourceEncoding);
    ts.setAutoDetectUnicode(true);

    inclusions.insert(cleanFile);
    if (isIndirect) {
        CppParser parser;
        for (const QString &projectRoot : std::as_const(cd.m_projectRoots))
            if (cleanFile.startsWith(projectRoot)) {
                parser.setTranslator(new Translator);
                break;
            }
        parser.setInput(ts, cleanFile);
        QStringList stack = includeStack;
        stack << cleanFile;
        parser.parse(cd, stack, inclusions);
        results->includes.insert(parser.recordResults(true));
    } else {
        CppParser parser(results);
        parser.namespaces = namespaces;
        parser.functionContext = functionContext;
        parser.functionContextUnresolved = functionContextUnresolved;
        parser.setInput(ts, cleanFile);
        parser.setTranslator(tor);
        QStringList stack = includeStack;
        stack << cleanFile;
        parser.parseInternal(cd, stack, inclusions);
        // Avoid that messages obtained by direct scanning are used
        CppFiles::setBlacklisted(cleanFile);
    }
    inclusions.remove(cleanFile);

    prospectiveContext.clear();
    pendingContext.clear();
}

/*
  The third part of this source file is the parser. It accomplishes
  a very easy task: It finds all strings inside a tr() or translate()
  call, and possibly finds out the context of the call. It supports
  three cases: (1) the context is specified, as in
  FunnyDialog::tr("Hello") or translate("FunnyDialog", "Hello");
  (2) the call appears within an inlined function; (3) the call
  appears within a function defined outside the class definition.
*/

bool CppParser::match(TokenType t)
{
    bool matches = (yyTok == t);
    if (matches)
        yyTok = getToken();
    return matches;
}

bool CppParser::matchString(QString *s)
{
    bool matches = false;
    s->clear();
    forever {
        if (yyTok != Tok_String && yyTok != Tok_RawString)
            return matches;
        matches = true;
        if (yyTok == Tok_String)
            *s += ParserTool::transcode(yyWord);
        else
            *s += yyWord;
        s->detach();
        yyTok = getToken();
    }
}

static const QString strQApplication = u"QApplication"_s;
static const QString strQCoreApplication = u"QCoreApplication"_s;
static const QString strUnicodeUTF8 = u"UnicodeUTF8"_s;
static const QString strDefaultCodec = u"DefaultCodec"_s;
static const QString strCodecForTr = u"CodecForTr"_s;
static const QString strLatin1 = u"Latin1"_s;

bool CppParser::matchEncoding()
{
    if (yyTok != Tok_Ident)
        return false;
    if (yyWord == strQApplication || yyWord == strQCoreApplication) {
        yyTok = getToken();
        if (yyTok == Tok_ColonColon)
            yyTok = getToken();
    }
    if (yyWord == strUnicodeUTF8) {
        yyTok = getToken();
        return true;
    }
    if (yyWord == strLatin1 || yyWord == strDefaultCodec || yyWord == strCodecForTr)
        yyMsg() << "Unsupported encoding Latin1/DefaultCodec/CodecForTr\n";
    return false;
}

bool CppParser::matchStringOrNull(QString *s)
{
    return matchString(s) || match(Tok_Null);
}

/*
 * match any expression that can return a number, which can be
 * 1. Literal number (e.g. '11')
 * 2. simple identifier (e.g. 'm_count')
 * 3. simple function call (e.g. 'size()' )
 * 4. function call on an object (e.g. 'list.size()')
 * 5. function call on an object (e.g. 'list->size()')
 *
 * Other cases:
 * size(2,4)
 * list().size()
 * list(a,b).size(2,4)
 * etc...
 */
bool CppParser::matchExpression()
{
    if (match(Tok_Null) || match(Tok_Integer))
        return true;

    int parenlevel = 0;
    while (match(Tok_Ident) || parenlevel > 0) {
        if (yyTok == Tok_RightParen) {
            if (parenlevel == 0) break;
            --parenlevel;
            yyTok = getToken();
        } else if (yyTok == Tok_LeftParen) {
            yyTok = getToken();
            if (yyTok == Tok_RightParen) {
                yyTok = getToken();
            } else {
                ++parenlevel;
            }
        } else if (yyTok == Tok_Ident) {
            continue;
        } else if (yyTok == Tok_Arrow) {
            yyTok = getToken();
        } else if (parenlevel == 0 || yyTok == Tok_Cancel) {
            return false;
        }
    }
    return true;
}

void CppParser::recordMessage(int line, const QString &context, const QString &text, const QString &comment,
    const QString &extracomment, const QString &msgid, const TranslatorMessage::ExtraData &extra, bool plural)
{
    TranslatorMessage msg(
        ParserTool::transcode(context), text, ParserTool::transcode(comment), QString(),
        yyFileName, line, QStringList(),
        TranslatorMessage::Unfinished, plural);
    msg.setExtraComment(ParserTool::transcode(extracomment.simplified()));
    msg.setId(msgid);
    msg.setExtras(extra);
    tor->append(msg);
}

void CppParser::handleTr(QString &prefix, bool plural)
{
    if (!sourcetext.isEmpty())
        yyMsg() << "//% cannot be used with tr() / QT_TR_NOOP(). Ignoring\n";
    int line = yyLineNo;
    yyTok = getToken();
    if (matchString(&text) && !text.isEmpty()) {
        comment.clear();

        if (yyTok == Tok_RightParen) {
            // no comment
        } else if (match(Tok_Comma) && matchStringOrNull(&comment)) {   //comment
            if (yyTok == Tok_RightParen) {
                // ok,
            } else if (match(Tok_Comma)) {
                plural = true;
            }
        }
        if (!pendingContext.isEmpty() && !prefix.startsWith(QLatin1String("::"))) {
            NamespaceList unresolved;
            if (!fullyQualify(namespaces, pendingContext, true, &functionContext, &unresolved)) {
                functionContextUnresolved = stringifyNamespace(0, unresolved);
                yyMsg() << qPrintable(
                    QStringLiteral("Qualifying with unknown namespace/class %1::%2\n")
                    .arg(stringifyNamespace(functionContext)).arg(unresolved.first().value()));
            }
            pendingContext.clear();
        }
        if (prefix.isEmpty()) {
            if (functionContextUnresolved.isEmpty()) {
                int idx = functionContext.size();
                if (idx < 2) {
                    yyMsg() << "tr() cannot be called without context\n";
                    return;
                }
                Namespace *fctx;
                while (!(fctx = findNamespace(functionContext, idx)->classDef)->hasTrFunctions) {
                    if (idx == 1) {
                        context = stringifyNamespace(functionContext);
                        fctx = findNamespace(functionContext)->classDef;
                        if (!fctx->complained) {
                            yyMsg() << qPrintable(
                                QStringLiteral("Class '%1' lacks Q_OBJECT macro\n").arg(context));
                            fctx->complained = true;
                        }
                        goto gotctx;
                    }
                    --idx;
                }
                if (fctx->trQualification.isEmpty()) {
                    context.clear();
                    for (int i = 1;;) {
                        context += functionContext.at(i).value();
                        if (++i == idx)
                            break;
                        context += QLatin1String("::");
                    }
                    fctx->trQualification = context;
                } else {
                    context = fctx->trQualification;
                }
            } else {
                context = joinNamespaces(stringifyNamespace(functionContext), functionContextUnresolved);
            }
        } else {
            prefix.chop(2);
            NamespaceList nsl;
            NamespaceList unresolved;
            if (fullyQualify(functionContext, prefix, false, &nsl, &unresolved)) {
                Namespace *fctx = findNamespace(nsl)->classDef;
                if (fctx->trQualification.isEmpty()) {
                    context = stringifyNamespace(nsl);
                    fctx->trQualification = context;
                } else {
                    context = fctx->trQualification;
                }
                if (!fctx->hasTrFunctions && !fctx->complained) {
                    yyMsg() << qPrintable(QStringLiteral("Class '%1' lacks Q_OBJECT macro\n")
                                          .arg(context));
                    fctx->complained = true;
                }
            } else {
                context = joinNamespaces(stringifyNamespace(nsl), stringifyNamespace(0, unresolved));
            }
            prefix.clear();
        }

      gotctx:
        recordMessage(line, context, text, comment, extracomment, msgid, extra, plural);
    }
    sourcetext.clear(); // Will have warned about that already
    extracomment.clear();
    msgid.clear();
    extra.clear();
    metaExpected = false;
}

void CppParser::handleTranslate(bool plural)
{
    if (!sourcetext.isEmpty())
        yyMsg() << "//% cannot be used with translate() / QT_TRANSLATE_NOOP(). Ignoring\n";
    int line = yyLineNo;
    yyTok = getToken();
    if (matchString(&context)
        && match(Tok_Comma)
        && matchString(&text) && !text.isEmpty())
    {
        comment.clear();
        if (yyTok != Tok_RightParen) {
            // look for comment
            if (match(Tok_Comma) && matchStringOrNull(&comment)) {
                if (yyTok != Tok_RightParen) {
                    // look for encoding
                    if (match(Tok_Comma)) {
                        if (matchEncoding()) {
                            if (yyTok != Tok_RightParen) {
                                // look for the plural quantifier,
                                // this can be a number, an identifier or
                                // a function call,
                                // so for simplicity we mark it as plural if
                                // we know we have a comma instead of an
                                // right parentheses.
                                plural |= match(Tok_Comma);
                            }
                        } else {
                            // This can be a QTranslator::translate("context",
                            // "source", "comment", n) plural translation
                            if (matchExpression() && yyTok == Tok_RightParen) {
                                plural = true;
                            } else {
                                return;
                            }
                        }
                    } else {
                        return;
                    }
                }
            } else {
                return;
            }
        }
        recordMessage(line, context, text, comment, extracomment, msgid, extra, plural);
    }
    sourcetext.clear(); // Will have warned about that already
    extracomment.clear();
    msgid.clear();
    extra.clear();
    metaExpected = false;
}

void CppParser::handleTrId(bool plural)
{
    if (!msgid.isEmpty())
        yyMsg() << "//= cannot be used with qtTrId() / QT_TRID_NOOP(). Ignoring\n";
    int line = yyLineNo;
    yyTok = getToken();
    if (matchString(&msgid) && !msgid.isEmpty()) {
        plural |= match(Tok_Comma);
        recordMessage(line, QString(), ParserTool::transcode(sourcetext), QString(), extracomment,
                      msgid, extra, plural);
    }
    sourcetext.clear();
    extracomment.clear();
    msgid.clear();
    extra.clear();
    metaExpected = false;
}

void CppParser::handleDeclareTrFunctions()
{
    QString name;
    forever {
        yyTok = getToken();
        if (yyTok != Tok_Ident)
            return;
        name += yyWord;
        name.detach();
        yyTok = getToken();
        if (yyTok == Tok_RightParen)
            break;
        if (yyTok != Tok_ColonColon)
            return;
        name += QLatin1String("::");
    }
    Namespace *ns = modifyNamespace(&namespaces);
    ns->hasTrFunctions = true;
    ns->trQualification = name;
    ns->trQualification.detach();
}

void CppParser::parse(ConversionData &cd, const QStringList &includeStack,
                      QSet<QString> &inclusions)
{
    namespaces << HashString();
    functionContext = namespaces;
    functionContextUnresolved.clear();

    parseInternal(cd, includeStack, inclusions);
}

void CppParser::parseInternal(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions)
{
    static QString strColons(QLatin1String("::"));

    QString prefix;
    bool yyTokColonSeen = false; // Start of c'tor's initializer list
    bool yyTokIdentSeen = false; // Start of initializer (member or base class)
    bool maybeInTrailingReturnType = false;
    metaExpected = true;

    prospectiveContext.clear();
    pendingContext.clear();

    yyWord.reserve(yyInStr.size()); // Rather insane. That's because we do no length checking.
    yyWordInitialCapacity = yyWord.capacity();
    yyInPtr = (const ushort *)yyInStr.unicode();
    yyCh = getChar();
    yyTok = getToken();
    while (yyTok != Tok_Eof) {
        // these are array indexing operations. we ignore them entirely
        // so they don't confuse our scoping of static initializers.
        // we enter the loop by either reading a left bracket or by an
        // #else popping the state.
        if (yyBracketDepth && yyBraceDepth == namespaceDepths.size()) {
            yyTok = getToken();
            continue;
        }
        //qDebug() << "TOKEN: " << yyTok;
        switch (yyTok) {
        case Tok_QuotedInclude: {
            text = QDir(QFileInfo(yyFileName).absolutePath()).absoluteFilePath(yyWord);
            text.detach();
            if (QFileInfo(text).isFile()) {
                processInclude(text, cd, includeStack, inclusions);
                yyTok = getToken();
                break;
            }
        }
        Q_FALLTHROUGH();
        case Tok_AngledInclude: {
            const QStringList cSources = cd.m_allCSources.values(yyWord);
            if (!cSources.isEmpty()) {
                for (const QString &cSource : cSources)
                    processInclude(cSource, cd, includeStack, inclusions);
                goto incOk;
            }
            for (const QString &incPath : std::as_const(cd.m_includePath)) {
                text = QDir(incPath).absoluteFilePath(yyWord);
                text.detach();
                if (QFileInfo(text).isFile()) {
                    processInclude(text, cd, includeStack, inclusions);
                    goto incOk;
                }
            }
          incOk:
            yyTok = getToken();
            break;
        }
        case Tok_friend:
            yyTok = getToken();
            // These are forward declarations, so ignore them.
            if (yyTok == Tok_class)
                yyTok = getToken();
            break;
        case Tok_class:
            /*
              Partial support for inlined functions.
            */
            yyTok = getToken();
            if (yyBraceDepth == namespaceDepths.size() && yyParenDepth == 0) {
                NamespaceList quali;
                HashString fct;

                // Find class name including qualification
                forever {
                    text = yyWord;
                    text.detach();
                    fct.setValue(text);
                    yyTok = getToken();

                    if (yyTok == Tok_ColonColon) {
                        quali << fct;
                        yyTok = getToken();
                    } else if (yyTok == Tok_Ident) {
                        if (yyWord == strfinal) {
                            // C++11: final may appear immediately after the name of the class
                            yyTok = getToken();
                            break;
                        }

                        // Handle impure definitions such as 'class Q_EXPORT QMessageBox', in
                        // which case 'QMessageBox' is the class name, not 'Q_EXPORT', by
                        // abandoning any qualification collected so far.
                        quali.clear();
                    } else {
                        break;
                    }
                }

                if (yyTok == Tok_Colon || yyTok == Tok_AngleBracket) {
                    // Skip any token until '{' or ';' since we might do things wrong if we find
                    // a '::' or ':' token here.
                    do {
                        yyTok = getToken();
                        if (yyTok == Tok_Eof)
                            goto goteof;
                        if (yyTok == Tok_Cancel)
                            goto case_default;
                    } while (yyTok != Tok_LeftBrace && yyTok != Tok_Semicolon);
                } else {
                    if (yyTok != Tok_LeftBrace) {
                        // Obviously a forward declaration. We skip those, as they
                        // don't create actually usable namespaces.
                        break;
                    }
                }

                if (!quali.isEmpty()) {
                    // Forward-declared class definitions can be namespaced.
                    NamespaceList nsl;
                    if (!fullyQualify(namespaces, quali, true, &nsl, 0)) {
                        yyMsg() << "Ignoring definition of undeclared qualified class\n";
                        break;
                    }
                    namespaceDepths.push(namespaces.size());
                    namespaces = nsl;
                } else {
                    namespaceDepths.push(namespaces.size());
                }
                enterNamespace(&namespaces, fct);

                functionContext = namespaces;
                functionContextUnresolved.clear(); // Pointless
                prospectiveContext.clear();
                pendingContext.clear();

                metaExpected = true;
                yyTok = getToken();
            }
            break;
        case Tok_namespace:
            yyTok = getToken();
            if (yyTok == Tok_Ident) {
                text = yyWord;
                text.detach();
                HashString ns = HashString(text);
                NamespaceList nestedNamespaces;
                forever {
                    yyTok = getToken();
                    if (yyTok != Tok_ColonColon)
                        break;
                    yyTok = getToken();
                    if (yyTok != Tok_Ident)
                        break;  // whoops
                    nestedNamespaces.append(ns);
                    text = yyWord;
                    text.detach();
                    ns = HashString(text);
                }
                if (yyTok == Tok_LeftBrace) {
                    namespaceDepths.push(namespaces.size());
                    for (const auto &nns : nestedNamespaces)
                        enterNamespace(&namespaces, nns);
                    enterNamespace(&namespaces, ns);

                    functionContext = namespaces;
                    functionContextUnresolved.clear();
                    prospectiveContext.clear();
                    pendingContext.clear();
                    metaExpected = true;
                    yyTok = getToken();
                } else if (yyTok == Tok_Equals) {
                    // e.g. namespace Is = OuterSpace::InnerSpace;
                    // Note: 'Is' being qualified is invalid per C++17.
                    NamespaceList fullName;
                    yyTok = getToken();
                    if (yyTok == Tok_ColonColon)
                        fullName.append(HashString(QString()));
                    while (yyTok == Tok_ColonColon || yyTok == Tok_Ident) {
                        if (yyTok == Tok_Ident) {
                            text = yyWord;
                            text.detach();
                            fullName.append(HashString(text));
                        }
                        yyTok = getToken();
                    }
                    if (fullName.isEmpty())
                        break;
                    fullName.append(HashString(QString())); // Mark as unresolved
                    modifyNamespace(&namespaces)->aliases[ns] = fullName;
                }
            } else if (yyTok == Tok_LeftBrace) {
                // Anonymous namespace
                namespaceDepths.push(namespaces.size());
                metaExpected = true;
                yyTok = getToken();
            }
            break;
        case Tok_using:
            yyTok = getToken();
            // XXX this should affect only the current scope, not the entire current namespace
            if (yyTok == Tok_namespace) {
                NamespaceList fullName;
                yyTok = getToken();
                if (yyTok == Tok_ColonColon)
                    fullName.append(HashString(QString()));
                while (yyTok == Tok_ColonColon || yyTok == Tok_Ident) {
                    if (yyTok == Tok_Ident) {
                        text = yyWord;
                        text.detach();
                        fullName.append(HashString(text));
                    }
                    yyTok = getToken();
                }
                NamespaceList nsl;
                if (fullyQualify(namespaces, fullName, false, &nsl, 0))
                    modifyNamespace(&namespaces)->usings << HashStringList(nsl);
            } else {
                NamespaceList fullName;
                if (yyTok == Tok_ColonColon)
                    fullName.append(HashString(QString()));
                while (yyTok == Tok_ColonColon || yyTok == Tok_Ident) {
                    if (yyTok == Tok_Ident) {
                        text = yyWord;
                        text.detach();
                        fullName.append(HashString(text));
                    }
                    yyTok = getToken();
                }
                if (fullName.isEmpty())
                    break;
                // using-declarations cannot rename classes, so the last element of
                // fullName is already the resolved name we actually want.
                // As we do no resolution here, we'll collect useless usings of data
                // members and methods as well. This is no big deal.
                fullName.append(HashString(QString())); // Mark as unresolved
                const HashString &ns = *(fullName.constEnd() - 2);
                modifyNamespace(&namespaces)->aliases[ns] = fullName;
            }
            break;
        case Tok_Q_OBJECT:
            modifyNamespace(&namespaces)->hasTrFunctions = true;
            yyTok = getToken();
            break;
        case Tok_Ident:
            if (yyTokColonSeen &&
                yyBraceDepth == namespaceDepths.size() && yyParenDepth == 0) {
                // member or base class identifier
                yyTokIdentSeen = true;
            }
            yyTok = getToken();
            if (yyTok == Tok_LeftParen) {
                bool forcePlural = false;
                switch (trFunctionAliasManager.trFunctionByName(yyWord)) {
                case TrFunctionAliasManager::Function_Q_DECLARE_TR_FUNCTIONS:
                    handleDeclareTrFunctions();
                    break;
                case TrFunctionAliasManager::Function_QT_TR_N_NOOP:
                    forcePlural = true;
                    Q_FALLTHROUGH();
                case TrFunctionAliasManager::Function_tr:
                case TrFunctionAliasManager::Function_trUtf8:
                case TrFunctionAliasManager::Function_QT_TR_NOOP:
                case TrFunctionAliasManager::Function_QT_TR_NOOP_UTF8:
                    if (tor)
                        handleTr(prefix, forcePlural);
                    break;
                case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP3:
                    forcePlural = true;
                    Q_FALLTHROUGH();
                case TrFunctionAliasManager::Function_translate:
                case TrFunctionAliasManager::Function_findMessage:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3:
                case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP3_UTF8:
                    if (tor)
                        handleTranslate(forcePlural);
                    break;
                case TrFunctionAliasManager::Function_QT_TRID_N_NOOP:
                    forcePlural = true;
                    Q_FALLTHROUGH();
                case TrFunctionAliasManager::Function_qtTrId:
                case TrFunctionAliasManager::Function_QT_TRID_NOOP:
                    if (tor)
                        handleTrId(forcePlural);
                    break;
                default:
                    goto notrfunc;
                }
                yyTok = getToken();
                break;
            }
            if (yyTok == Tok_ColonColon && !maybeInTrailingReturnType) {
                prefix += yyWord;
                prefix.detach();
            } else {
              notrfunc:
                prefix.clear();
            }
            metaExpected = false;
            break;
        case Tok_Arrow:
            if (yyParenDepth == 0 && yyBraceDepth == namespaceDepths.size())
                maybeInTrailingReturnType = true;
            yyTok = getToken();
            if (yyTok == Tok_Ident) {
                switch (trFunctionAliasManager.trFunctionByName(yyWord)) {
                case TrFunctionAliasManager::Function_tr:
                case TrFunctionAliasManager::Function_trUtf8:
                    yyMsg() << "Cannot invoke tr() like this\n";
                    break;
                }
            }
            break;
        case Tok_ColonColon:
            if (yyTokIdentSeen || maybeInTrailingReturnType) {
                // member or base class identifier
                yyTok = getToken();
                break;
            }
            if (yyBraceDepth == namespaceDepths.size() && yyParenDepth == 0 && !yyTokColonSeen)
                prospectiveContext = prefix;
            prefix += strColons;
            yyTok = getToken();
            break;
        case Tok_RightBrace:
            if (!yyTokColonSeen) {
                if (yyBraceDepth + 1 == namespaceDepths.size()) {
                    // class or namespace
                    truncateNamespaces(&namespaces, namespaceDepths.pop());
                }
                if (yyBraceDepth == namespaceDepths.size()) {
                    // function, class or namespace
                    if (!yyBraceDepth && !directInclude)
                        truncateNamespaces(&functionContext, 1);
                    else
                        functionContext = namespaces;
                    functionContextUnresolved.clear();
                    pendingContext.clear();
                }
            }
            Q_FALLTHROUGH();
        case Tok_Semicolon:
            maybeInTrailingReturnType = false;
            prospectiveContext.clear();
            prefix.clear();
            if (!sourcetext.isEmpty() || !extracomment.isEmpty() || !msgid.isEmpty() || !extra.isEmpty()) {
                yyMsg() << "Discarding unconsumed meta data\n";
                sourcetext.clear();
                extracomment.clear();
                msgid.clear();
                extra.clear();
            }
            metaExpected = true;
            yyTok = getToken();
            break;
        case Tok_Access:
            // Eat access specifiers, so their colons are not mistaken for c'tor initializer list starts
            do {
                yyTok = getToken();
            } while (yyTok == Tok_Access); // Multiple specifiers are possible, e.g. "public slots"
            metaExpected = true;
            if (yyTok == Tok_Colon)
                goto case_default;
            break;
        case Tok_Colon:
        case Tok_Equals:
            if (yyBraceDepth == namespaceDepths.size() && yyParenDepth == 0) {
                if (!prospectiveContext.isEmpty()) {
                    pendingContext = prospectiveContext;
                    prospectiveContext.clear();
                }
                //ignore colons for bitfields (are usually followed by a semicolon)
                if (yyTok == Tok_Colon) {
                    if (lookAheadToSemicolonOrLeftBrace() != Tok_Semicolon)
                        yyTokColonSeen = true;
                }
            }
            metaExpected = true;
            yyTok = getToken();
            break;
        case Tok_LeftBrace:
            if (yyBraceDepth == namespaceDepths.size() + 1 && yyParenDepth == 0) {
                if (!prospectiveContext.isEmpty()) {
                    pendingContext = prospectiveContext;
                    prospectiveContext.clear();
                }
                if (!yyTokIdentSeen) {
                    // Function body
                    yyTokColonSeen = false;
                }
            }
            maybeInTrailingReturnType = false;
            yyTokIdentSeen = false;
            metaExpected = true;
            yyTok = getToken();
            break;
        case Tok_LeftParen:
            if (!yyTokColonSeen && yyBraceDepth == namespaceDepths.size() && yyParenDepth == 1
                && !prospectiveContext.isEmpty()) {
                pendingContext = prospectiveContext;
                prospectiveContext.clear();
            }
            yyTokIdentSeen = false;
            metaExpected = true;
            yyTok = getToken();
            break;
        case Tok_Comma:
        case Tok_QuestionMark:
            metaExpected = true;
            yyTok = getToken();
            break;
        case Tok_RightParen:
            if (yyParenDepth == 0) {
                if (!yyTokColonSeen && !pendingContext.isEmpty()
                    && yyBraceDepth == namespaceDepths.size()) {
                    // Demote the pendingContext to prospectiveContext.
                    prospectiveContext = pendingContext;
                    pendingContext.clear();
                }
                metaExpected = true;
            } else {
                metaExpected = false;
            }
            yyTok = getToken();
            break;
        case Tok_decltype:
            {
                // Save the parentheses depth outside the 'decltype' specifier.
                auto initialParenDepth = yyParenDepth;

                // Eat the opening parenthesis that follows 'decltype'.
                yyTok = getToken();

                // Skip over everything within the parentheses that follow 'decltype'.
                while (yyParenDepth != initialParenDepth && yyTok != Tok_Eof)
                    yyTok = getToken();
            }
            break;
        case Tok_enum:
            yyTok = getToken();
            // If it is an enum class then ignore
            if (yyTok == Tok_class)
                yyTok = getToken();
            break;
        default:
            if (!yyParenDepth && !maybeInTrailingReturnType)
                prospectiveContext.clear();
            Q_FALLTHROUGH();
        case Tok_RightBracket: // ignoring indexing; for static initializers
        case_default:
            yyTok = getToken();
            break;
        }
    }

  goteof:
    if (yyBraceDepth != 0)
        yyMsg(yyBraceLineNo)
            << "Unbalanced opening brace in C++ code (or abuse of the C++ preprocessor)\n";
    else if (yyParenDepth != 0)
        yyMsg(yyParenLineNo)
            << "Unbalanced opening parenthesis in C++ code"
               " (or abuse of the C++ preprocessor)\n";
    else if (yyBracketDepth != 0)
        yyMsg(yyBracketLineNo)
            << "Unbalanced opening bracket in C++ code"
               " (or abuse of the C++ preprocessor)\n";
}

void CppParser::processComment()
{
    if (!tor || !metaExpected)
        return;

    const QChar *ptr = yyWord.unicode();
    if (*ptr == QLatin1Char(':') && ptr[1].isSpace()) {
        yyWord.remove(0, 2);
        extracomment += yyWord;
        extracomment.detach();
    } else if (*ptr == QLatin1Char('=') && ptr[1].isSpace()) {
        yyWord.remove(0, 2);
        msgid = yyWord.simplified();
        msgid.detach();
    } else if (*ptr == QLatin1Char('~') && ptr[1].isSpace()) {
        yyWord.remove(0, 2);
        text = yyWord.trimmed();
        int k = text.indexOf(QLatin1Char(' '));
        if (k > -1) {
            QString commentvalue = text.mid(k + 1).trimmed();
            if (commentvalue.startsWith(QLatin1Char('"')) && commentvalue.endsWith(QLatin1Char('"'))
                && commentvalue.size() != 1) {
                commentvalue = commentvalue.sliced(1, commentvalue.size() - 2);
            }
            extra.insert(text.left(k), commentvalue);
        }
        text.clear();
    } else if (*ptr == QLatin1Char('%') && ptr[1].isSpace()) {
        sourcetext.reserve(sourcetext.size() + yyWord.size() - 2);
        ushort *ptr = (ushort *)sourcetext.data() + sourcetext.size();
        int p = 2, c;
        forever {
            if (p >= yyWord.size())
                break;
            c = yyWord.unicode()[p++].unicode();
            if (isspace(c))
                continue;
            if (c != '"') {
                yyMsg() << "Unexpected character in meta string\n";
                break;
            }
            forever {
                if (p >= yyWord.size()) {
                  whoops:
                    yyMsg() << "Unterminated meta string\n";
                    break;
                }
                c = yyWord.unicode()[p++].unicode();
                if (c == '"')
                    break;
                if (c == '\\') {
                    if (p >= yyWord.size())
                        goto whoops;
                    c = yyWord.unicode()[p++].unicode();
                    if (c == '\n')
                        goto whoops;
                    *ptr++ = '\\';
                }
                *ptr++ = c;
            }
        }
        sourcetext.resize(ptr - (ushort *)sourcetext.data());
    } else {
        const ushort *uc = (const ushort *)yyWord.unicode(); // Is zero-terminated
        int idx = 0;
        ushort c;
        while ((c = uc[idx]) == ' ' || c == '\t' || c == '\n')
            ++idx;
        if (!memcmp(uc + idx, CppMagicComment.unicode(), CppMagicComment.size() * 2)) {
            idx += CppMagicComment.size();
            comment = QString::fromRawData(yyWord.unicode() + idx,
                                           yyWord.size() - idx).simplified();
            int k = comment.indexOf(QLatin1Char(' '));
            if (k == -1) {
                context = comment;
            } else {
                context = comment.left(k);
                comment.remove(0, k + 1);
                TranslatorMessage msg(
                        ParserTool::transcode(context), QString(),
                        ParserTool::transcode(comment), QString(),
                        yyFileName, yyLineNo, QStringList(),
                        TranslatorMessage::Finished, false);
                msg.setExtraComment(ParserTool::transcode(extracomment.simplified()));
                extracomment.clear();
                tor->append(msg);
                tor->setExtras(extra);
                extra.clear();
            }
        }
    }
}

const ParseResults *CppParser::recordResults(bool isHeader)
{
    if (tor) {
        if (tor->messageCount()) {
            CppFiles::setTranslator(yyFileName, tor);
        } else {
            delete tor;
            tor = 0;
        }
    }
    if (isHeader) {
        const ParseResults *pr;
        if (!tor && results->includes.size() == 1
            && results->rootNamespace.children.isEmpty()
            && results->rootNamespace.aliases.isEmpty()
            && results->rootNamespace.usings.isEmpty()) {
            // This is a forwarding header. Slash it.
            pr = *results->includes.cbegin();
            delete results;
        } else {
            results->fileId = nextFileId++;
            pr = results;
        }
        CppFiles::setResults(ResultsCacheKey(yyFileName, *this), pr);
        return pr;
    } else {
        delete results;
        return 0;
    }
}

void loadCPP(Translator &translator, const QStringList &filenames, ConversionData &cd)
{
    QStringConverter::Encoding e = cd.m_sourceIsUtf16 ? QStringConverter::Utf16 : QStringConverter::Utf8;

    for (const QString &filename : filenames) {
        if (!CppFiles::getResults(ResultsCacheKey(filename)).isEmpty() || CppFiles::isBlacklisted(filename))
            continue;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            cd.appendError(QStringLiteral("Cannot open %1: %2").arg(filename,
                                                                    file.errorString()));
            continue;
        }

        CppParser parser;
        QTextStream ts(&file);
        ts.setEncoding(e);
        ts.setAutoDetectUnicode(true);
        parser.setInput(ts, filename);
        Translator *tor = new Translator;
        parser.setTranslator(tor);
        QSet<QString> inclusions;
        parser.parse(cd, QStringList(), inclusions);
        parser.recordResults(isHeader(filename));
    }

    for (const QString &filename : filenames) {
        if (!CppFiles::isBlacklisted(filename)) {
            if (const Translator *tor = CppFiles::getTranslator(filename)) {
                for (const TranslatorMessage &msg : tor->messages())
                    translator.extend(msg, cd);
            }
        }
    }
}

QT_END_NAMESPACE
