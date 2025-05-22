// Copyright (C) 2002-2007 Detlev Offenbach <detlev@die-offenbachs.de>
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <translator.h>
#include "lupdate.h"
#include "metastrings.h"

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstack.h>

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

class PythonParser
{

    enum Token {
        Tok_Eof,
        Tok_class,
        Tok_def,
        Tok_return,
        Tok_tr,
        Tok_trUtf8,
        Tok_translate,
        Tok_Ident,
        Tok_Dot,
        Tok_String,
        Tok_LeftParen,
        Tok_RightParen,
        Tok_Comma,
        Tok_None,
        Tok_Integer
    };

    enum class StringType { NoString, String, FormatString, RawString };

public:
    PythonParser(Translator &translator, const QString &fileName, bool &error, ConversionData &cd)
        : tor(translator), m_cd(cd)
    {
#ifdef Q_CC_MSVC
        const auto *fileNameC = reinterpret_cast<const wchar_t *>(fileName.utf16());
        error = _wfopen_s(&yyInFile, fileNameC, L"r") != 0;
#else
        const QByteArray fileNameC = QFile::encodeName(fileName);
        yyInFile = std::fopen(fileNameC.constData(), "r");
        error = yyInFile == nullptr;
#endif
        if (!error)
            startTokenizer(fileName);
    }

    /*
      Accomplishes a very easy task: It finds all strings inside a tr() or translate()
      call, and possibly finds out the context of the call. It supports
      three cases:
      (1) the context is specified, as in FunnyDialog.tr("Hello") or
         translate("FunnyDialog", "Hello");
      (2) the call appears within an inlined function;
      (3) the call appears within a function defined outside the class definition.
    */
    void parse(const QByteArray &initialContext = {}, const QByteArray &defaultContext = {})
    {
        QByteArray context;
        QByteArray text;
        QByteArray comment;
        QByteArray prefix;
        bool utf8 = false;

        yyTok = getToken();
        while (yyTok != Tok_Eof) {

            switch (yyTok) {
            case Tok_class: {
                if (yyIndentationSize < 0 && yyContinuousSpaceCount > 0)
                    yyIndentationSize = yyContinuousSpaceCount; // First indented "class"
                const int indent =
                        yyIndentationSize > 0 ? yyContinuousSpaceCount / yyIndentationSize : 0;
                while (!yyContextStack.isEmpty() && yyContextStack.top().second >= indent)
                    yyContextStack.pop();
                yyTok = getToken();
                yyContextStack.push({ yyIdent, indent });
                yyTok = getToken();
            } break;
            case Tok_def:
                if (yyIndentationSize < 0 && yyContinuousSpaceCount > 0)
                    yyIndentationSize = yyContinuousSpaceCount; // First indented "def"
                if (!yyContextStack.isEmpty()) {
                    // Pop classes if the function is further outdented than the class on the top
                    // (end of a nested class).
                    const int classIndent = yyIndentationSize > 0
                            ? yyContinuousSpaceCount / yyIndentationSize - 1
                            : 0;
                    while (!yyContextStack.isEmpty() && yyContextStack.top().second > classIndent)
                        yyContextStack.pop();
                }
                yyTok = getToken();
                break;
            case Tok_tr:
            case Tok_trUtf8: {
                utf8 = true;
                yyTok = getToken();
                const int lineNo = yyCurLineNo;
                if (match(Tok_LeftParen) && matchString(&text)) {
                    comment.clear();
                    bool plural = false;

                    MetaStrings metaBackup = std::move(metaStrings);

                    if (match(Tok_RightParen)) {
                        // There is no comment or plural arguments.
                    } else if (match(Tok_Comma) && matchStringOrNone(&comment)) {
                        // There is a comment argument.
                        if (match(Tok_RightParen)) {
                            // There is no plural argument.
                        } else if (match(Tok_Comma)) {
                            // There is a plural argument.
                            plural = true;
                        }
                    }

                    if (prefix.isEmpty())
                        context = defaultContext;
                    else if (prefix == "self")
                        context = yyContextStack.isEmpty() ? initialContext
                                                           : yyContextStack.top().first;
                    else
                        context = prefix;

                    prefix.clear();
                    TranslatorMessage message(QString::fromUtf8(context), QString::fromUtf8(text),
                                              QString::fromUtf8(comment), {}, yyFileName, lineNo,
                                              {}, TranslatorMessage::Unfinished, plural);
                    setMessageParameters(&message, metaBackup);
                    tor.extend(message, m_cd);
                }
            } break;
            case Tok_translate: {
                bool plural{};
                const int lineNo = yyCurLineNo;
                MetaStrings metaBackup = std::move(metaStrings);
                if (parseTranslate(&text, &context, &comment, &utf8, &plural)) {
                    TranslatorMessage message(QString::fromUtf8(context), QString::fromUtf8(text),
                                              QString::fromUtf8(comment), {}, yyFileName, lineNo,
                                              {}, TranslatorMessage::Unfinished, plural);
                    setMessageParameters(&message, metaBackup);
                    tor.extend(message, m_cd);
                } else {
                    metaStrings = std::move(metaBackup);
                }
            } break;
            case Tok_Ident:
                if (!prefix.isEmpty())
                    prefix += '.';
                prefix += yyIdent;
                yyTok = getToken();
                if (yyTok != Tok_Dot)
                    prefix.clear();
                break;
            default:
                yyTok = getToken();
            }
        }

        if (yyParenDepth != 0) {
            qWarning("%s: Unbalanced parentheses in Python code", qPrintable(yyFileName));
        }
    }

    ~PythonParser() { std::fclose(yyInFile); }

private:
    QHash<QByteArray, Token> fillTokens()
    {
        QHash<QByteArray, Token> tokens = { { "None", Tok_None },      { "class", Tok_class },
                                            { "def", Tok_def },        { "return", Tok_return },
                                            { "__tr", Tok_tr }, // Legacy?
                                            { "__trUtf8", Tok_trUtf8 } };

        const auto &nameMap = trFunctionAliasManager.nameToTrFunctionMap();
        for (auto it = nameMap.cbegin(), end = nameMap.cend(); it != end; ++it) {
            switch (it.value()) {
            case TrFunctionAliasManager::Function_tr:
            case TrFunctionAliasManager::Function_QT_TR_NOOP:
                tokens.insert(it.key().toUtf8(), Tok_tr);
                break;
            case TrFunctionAliasManager::Function_trUtf8:
                tokens.insert(it.key().toUtf8(), Tok_trUtf8);
                break;
            case TrFunctionAliasManager::Function_translate:
            case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
            // QTranslator::findMessage() has the same parameters as QApplication::translate().
            case TrFunctionAliasManager::Function_findMessage:
                tokens.insert(it.key().toUtf8(), Tok_translate);
                break;
            default:
                break;
            }
        }
        return tokens;
    }

    QHash<QByteArray, Token> &getTokens()
    {
        static QHash<QByteArray, Token> tokens = fillTokens();
        return tokens;
    }

    int getChar()
    {
        int c;

        if (buf < 0) {
            c = getc(yyInFile);
        } else {
            c = buf;
            buf = -1;
        }
        if (c == '\n') {
            yyCurLineNo++;
            yyCountingIndentation = true;
            yyContinuousSpaceCount = 0;
        } else if (yyCountingIndentation && (c == 32 || c == 9)) {
            yyContinuousSpaceCount++;
        } else {
            yyCountingIndentation = false;
        }
        return c;
    }

    int peekChar()
    {
        int c = getc(yyInFile);
        buf = c;
        return c;
    }

    void startTokenizer(const QString &fileName)
    {
        yyInPos = 0;
        buf = -1;

        yyFileName = fileName;
        yyCh = getChar();
        yyParenDepth = 0;
        yyCurLineNo = 1;

        yyIndentationSize = -1;
        yyContinuousSpaceCount = 0;
        yyContextStack.clear();
    }

    bool parseStringEscape(int quoteChar, StringType stringType)
    {
        static const char tab[] = "abfnrtv";
        static const char backTab[] = "\a\b\f\n\r\t\v";

        yyCh = getChar();
        if (yyCh == EOF)
            return false;

        if (stringType == StringType::RawString) {
            if (yyCh != quoteChar) // Only quotes can be escaped in raw strings
                yyString[yyStringLen++] = '\\';
            yyString[yyStringLen++] = yyCh;
            yyCh = getChar();
            return true;
        }

        if (yyCh == 'x' || yyCh == 'u' || yyCh == 'U') {
            qsizetype maxSize = 2; // \x
            if (yyCh == 'u')
                maxSize = 4;
            else if (yyCh == 'U')
                maxSize = 8;

            QByteArray hex;
            yyCh = getChar();
            if (yyCh == EOF)
                return false;

            while (maxSize-- && std::isxdigit(yyCh)) {
                hex += char(yyCh);
                yyCh = getChar();
                if (yyCh == EOF)
                    return false;
            }
            uint n;
#ifdef Q_CC_MSVC
            sscanf_s(hex, "%x", &n);
#else
            std::sscanf(hex, "%x", &n);
#endif

            QByteArray hexChar = QString(QChar(n)).toUtf8();
            if (yyStringLen < sizeof(yyString) - hexChar.size())
                for (char c : std::as_const(hexChar))
                    yyString[yyStringLen++] = c;
            return true;
        }

        if (yyCh >= '0' && yyCh < '8') {
            QByteArray oct;
            int n = 0;
            do {
                oct += char(yyCh);
                ++n;
                yyCh = getChar();
                if (yyCh == EOF)
                    return false;
            } while (yyCh >= '0' && yyCh < '8' && n < 3);
#ifdef Q_CC_MSVC
            sscanf_s(oct, "%o", &n);
#else
            std::sscanf(oct, "%o", &n);
#endif
            if (yyStringLen < sizeof(yyString) - 1)
                yyString[yyStringLen++] = char(n);
            return true;
        }

        const char *p = std::strchr(tab, yyCh);
        if (yyStringLen < sizeof(yyString) - 1) {
            yyString[yyStringLen++] = p == nullptr ? char(yyCh) : backTab[p - tab];
        }
        yyCh = getChar();
        return true;
    }

    Token parseString(StringType stringType = StringType::NoString)
    {
        int quoteChar = yyCh;
        bool tripleQuote = false;
        bool singleQuote = true;
        bool in = false;

        yyCh = getChar();

        while (yyCh != EOF) {
            if (singleQuote && (yyCh == '\n' || (in && yyCh == quoteChar)))
                break;

            if (yyCh == quoteChar) {
                if (peekChar() == quoteChar) {
                    yyCh = getChar();
                    if (!tripleQuote) {
                        tripleQuote = true;
                        singleQuote = false;
                        in = true;
                        yyCh = getChar();
                    } else {
                        yyCh = getChar();
                        if (yyCh == quoteChar) {
                            tripleQuote = false;
                            break;
                        }
                    }
                } else if (tripleQuote) {
                    if (yyStringLen < sizeof(yyString) - 1)
                        yyString[yyStringLen++] = char(yyCh);
                    yyCh = getChar();
                    continue;
                } else {
                    break;
                }
            } else {
                in = true;
            }

            if (yyCh == '\\') {
                if (!parseStringEscape(quoteChar, stringType))
                    return Tok_Eof;
            } else {
                char *yStart = yyString + yyStringLen;
                char *yp = yStart;
                while (yyCh != EOF && (tripleQuote || yyCh != '\n') && yyCh != quoteChar
                       && yyCh != '\\') {
                    *yp++ = char(yyCh);
                    yyCh = getChar();
                }
                yyStringLen += yp - yStart;
            }
        }
        yyString[yyStringLen] = '\0';

        if (yyCh != quoteChar) {
            printf("%c\n", yyCh);

            qWarning("%s:%d: Unterminated string", qPrintable(yyFileName), yyLineNo);
        }

        if (yyCh == EOF)
            return Tok_Eof;
        yyCh = getChar();
        return Tok_String;
    }

    QByteArray readLine()
    {
        QByteArray result;
        while (true) {
            yyCh = getChar();
            if (yyCh == EOF || yyCh == '\n')
                break;
            result.append(char(yyCh));
        }
        return result;
    }

    Token getToken(StringType stringType = StringType::NoString)
    {
        yyIdent.clear();
        yyStringLen = 0;
        while (yyCh != EOF) {
            yyLineNo = yyCurLineNo;

            if (std::isalpha(yyCh) || yyCh == '_') {
                do {
                    yyIdent.append(char(yyCh));
                    yyCh = getChar();
                } while (std::isalnum(yyCh) || yyCh == '_');

                return getTokens().value(yyIdent, Tok_Ident);
            }
            switch (yyCh) {
            case '#': {
                auto comment = QString::fromUtf8(readLine());
                if (!metaStrings.parse(comment)) {
                    qWarning() << qPrintable(yyFileName) << ':' << yyLineNo << ": "
                               << metaStrings.popError().toStdString();
                    break;
                }
                if (metaStrings.magicComment()) {
                    auto [context, comment] = *metaStrings.magicComment();
                    TranslatorMessage msg(ParserTool::transcode(context), QString(),
                                          ParserTool::transcode(comment), QString(), yyFileName,
                                          yyCurLineNo, QStringList(), TranslatorMessage::Finished,
                                          false);
                    msg.setExtraComment(
                            ParserTool::transcode(metaStrings.extracomment().simplified()));
                    tor.append(msg);
                    tor.setExtras(metaStrings.extra());
                    metaStrings.clear();
                }
                break;
            }
            case '"':
            case '\'':
                return parseString(stringType);
            case '(':
                yyParenDepth++;
                yyCh = getChar();
                return Tok_LeftParen;
            case ')':
                yyParenDepth--;
                yyCh = getChar();
                return Tok_RightParen;
            case ',':
                yyCh = getChar();
                return Tok_Comma;
            case '.':
                yyCh = getChar();
                return Tok_Dot;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                QByteArray ba;
                ba += char(yyCh);
                yyCh = getChar();
                const bool hex = yyCh == 'x';
                if (hex) {
                    ba += char(yyCh);
                    yyCh = getChar();
                }
                while ((hex ? std::isxdigit(yyCh) : std::isdigit(yyCh))) {
                    ba += char(yyCh);
                    yyCh = getChar();
                }
                bool ok;
                auto v = ba.toLongLong(&ok);
                Q_UNUSED(v);
                if (ok)
                    return Tok_Integer;
                break;
            }
            default:
                yyCh = getChar();
            }
        }
        return Tok_Eof;
    }

    bool match(Token t)
    {
        const bool matches = (yyTok == t);
        if (matches)
            yyTok = getToken();
        return matches;
    }

    bool matchStringStart()
    {
        if (yyTok == Tok_String)
            return true;
        // Check for f"bla{var}" and raw strings r"bla".
        if (yyTok == Tok_Ident && yyIdent.size() == 1) {
            switch (yyIdent.at(0)) {
            case 'r':
                yyTok = getToken(StringType::RawString);
                return yyTok == Tok_String;
            case 'f':
                yyTok = getToken(StringType::FormatString);
                return yyTok == Tok_String;
            }
        }
        return false;
    }

    bool matchString(QByteArray *s)
    {
        s->clear();
        bool ok = false;
        while (matchStringStart()) {
            *s += yyString;
            yyTok = getToken();
            ok = true;
        }
        return ok;
    }

    bool matchEncoding(bool *utf8)
    {
        // Remove any leading module paths.
        if (yyTok == Tok_Ident && std::strcmp(yyIdent, "PySide6") == 0) {
            yyTok = getToken();

            if (yyTok != Tok_Dot)
                return false;

            yyTok = getToken();
        }

        if (yyTok == Tok_Ident
            && (std::strcmp(yyIdent, "QtGui") == 0 || std::strcmp(yyIdent, "QtCore") == 0)) {
            yyTok = getToken();

            if (yyTok != Tok_Dot)
                return false;

            yyTok = getToken();
        }

        if (yyTok == Tok_Ident) {
            if (std::strcmp(yyIdent, "QApplication") == 0
                || std::strcmp(yyIdent, "QGuiApplication") == 0
                || std::strcmp(yyIdent, "QCoreApplication") == 0) {
                yyTok = getToken();

                if (yyTok == Tok_Dot)
                    yyTok = getToken();
            }

            *utf8 = QByteArray(yyIdent).endsWith("UTF8");
            yyTok = getToken();
            return true;
        }
        return false;
    }

    bool matchStringOrNone(QByteArray *s)
    {
        bool matches = matchString(s);

        if (!matches)
            matches = match(Tok_None);

        return matches;
    }

    /*
     * match any expression that can return a number, which can be
     * 1. Literal number (e.g. '11')
     * 2. simple identifier (e.g. 'm_count')
     * 3. simple function call (e.g. 'size()')
     * 4. function call on an object (e.g. 'list.size()')
     *      * Other cases:
     * size(2,4)
     * list().size()
     * list(a,b).size(2,4)
     * etc...
     */
    bool matchExpression()
    {
        if (match(Tok_Integer))
            return true;

        int parenlevel = 0;
        while (match(Tok_Ident) || parenlevel > 0) {
            if (yyTok == Tok_RightParen) {
                if (parenlevel == 0)
                    break;
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
            } else if (parenlevel == 0) {
                return false;
            }
        }
        return true;
    }

    bool parseTranslate(QByteArray *text, QByteArray *context, QByteArray *comment, bool *utf8,
                        bool *plural)
    {
        text->clear();
        context->clear();
        comment->clear();
        *utf8 = false;
        *plural = false;

        yyTok = getToken();
        if (!match(Tok_LeftParen) || !matchString(context) || !match(Tok_Comma)
            || !matchString(text)) {
            return false;
        }

        if (match(Tok_RightParen))
            return true;

        // not a comma or a right paren, illegal syntax
        if (!match(Tok_Comma))
            return false;

        // python accepts trailing commas within parenthesis, so allow a comma with nothing after
        if (match(Tok_RightParen))
            return true;

        // check for comment
        if (!matchStringOrNone(comment))
            return false; // not a comment, or a trailing comma... something is wrong

        if (match(Tok_RightParen))
            return true;

        // not a comma or a right paren, illegal syntax
        if (!match(Tok_Comma))
            return false;

        // python accepts trailing commas within parenthesis, so allow a comma with nothing after
        if (match(Tok_RightParen))
            return true;

        // look for optional encoding information
        if (matchEncoding(utf8)) {
            if (match(Tok_RightParen))
                return true;

            // not a comma or a right paren, illegal syntax
            if (!match(Tok_Comma))
                return false;

            // python accepts trailing commas within parenthesis, so allow a comma with nothing
            // after
            if (match(Tok_RightParen))
                return true;
        }

        // Must be a plural expression
        if (!matchExpression())
            return false;

        *plural = true;

        // Ignore any trailing comma here
        match(Tok_Comma);

        // This must be the end, or there are too many parameters
        if (match(Tok_RightParen))
            return true;

        return false;
    }

    void setMessageParameters(TranslatorMessage *message, const MetaStrings &meta)
    {
        // PYSIDE-2863: parseTranslate() can read past the message
        // and capture extraComments intended for the next message.
        // Use only extraComments for the current message.

        message->setExtraComment(ParserTool::transcode(meta.extracomment().simplified()));
        message->setId(meta.msgid());
        message->setExtras(meta.extra());
        if (!meta.label().isEmpty() && meta.msgid().isEmpty())
            m_cd.appendError("%1:%2: labels cannot be used with text-based translation. "
                             "Ignoring\n"_L1.arg(yyFileName)
                                     .arg(yyLineNo));
        else
            message->setLabel(meta.label());
    }

    QString yyFileName;
    Token yyTok{};
    int yyCh{};
    QByteArray yyIdent;
    char yyString[65536];
    size_t yyStringLen{};
    int yyParenDepth{};
    int yyLineNo = 1;
    int yyCurLineNo{};
    // the file to read from (if reading from a file)
    FILE *yyInFile;
    // the string to read from and current position in the string (otherwise)
    int yyInPos{};
    int buf{};
    int yyIndentationSize{};
    int yyContinuousSpaceCount{};
    bool yyCountingIndentation = false;
    // (Context, indentation level) pair.
    using ContextPair = QPair<QByteArray, int>;
    // Stack of (Context, indentation level) pairs.
    using ContextStack = QStack<ContextPair>;
    ContextStack yyContextStack;
    MetaStrings metaStrings;
    Translator &tor;
    ConversionData &m_cd;
};

bool loadPython(Translator &translator, const QString &fileName, ConversionData &cd)
{

    bool error = false;
    PythonParser parser(translator, fileName, error, cd);
    if (error) {
        cd.appendError(QStringLiteral("Cannot open %1").arg(fileName));
        return false;
    }

    parser.parse();
    return true;
}

QT_END_NAMESPACE
