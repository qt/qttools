// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlcodeparser.h"

#include "node.h"
#include "qmlvisitor.h"
#include "utilities.h"

#include <private/qqmljsast_p.h>

#include <QtCore/qstringview.h>
#include <qdebug.h>

#include <optional>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
  Returns "QML".
 */
QString QmlCodeParser::language()
{
    return "QML";
}

/*!
  Returns a string list containing "*.qml". This is the only
  file type parsed by the QMLN parser.
 */
QStringList QmlCodeParser::sourceFileNameFilter()
{
    return QStringList() << "*.qml";
}

/*!
  Parses the source file at \a filePath and inserts the contents
  into the database. The \a location is used for error reporting.

  If it can't open the file at \a filePath, it reports an error
  and returns without doing anything.
 */
void QmlCodeParser::parseSourceFile(const Location &location, const QString &filePath, CppCodeParser&)
{
    static const QSet<QString> topic_commands{
        COMMAND_VARIABLE, COMMAND_QMLCLASS, COMMAND_QMLTYPE, COMMAND_QMLSINGLETONTYPE, COMMAND_QMLPROPERTY,
        COMMAND_QMLPROPERTYGROUP, COMMAND_QMLATTACHEDPROPERTY, COMMAND_QMLSIGNAL,
        COMMAND_QMLATTACHEDSIGNAL, COMMAND_QMLMETHOD, COMMAND_QMLATTACHEDMETHOD,
        COMMAND_QMLVALUETYPE, COMMAND_QMLBASICTYPE,
    };

    QFile in(filePath);
    if (!in.open(QIODevice::ReadOnly)) {
        location.error(QStringLiteral("Cannot open QML file '%1'").arg(filePath));
        return;
    }

    QString document = in.readAll();
    in.close();

    QString newCode = std::move(document);
    extractPragmas(newCode);

    QQmlJS::Engine engine{};
    QQmlJS::Lexer lexer{&engine};
    lexer.setCode(newCode, 1);

    QQmlJS::Parser parser{&engine};

    if (parser.parse()) {
        QQmlJS::AST::UiProgram *ast = parser.ast();
        QmlDocVisitor visitor(filePath, newCode, &engine, topic_commands + CodeParser::common_meta_commands,
                              topic_commands);
        QQmlJS::AST::Node::accept(ast, &visitor);
        if (visitor.hasError())
            Location(filePath).warning("Could not analyze QML file, output is incomplete.");
    }
    const auto &messages = parser.diagnosticMessages();
    for (const auto &msg : messages) {
        qCDebug(lcQdoc, "%s: %d: %d: QML syntax error: %s", qUtf8Printable(filePath),
                msg.loc.startLine, msg.loc.startColumn, qUtf8Printable(msg.message));
    }
}

/*!
  Copy and paste from src/declarative/qml/qdeclarativescriptparser.cpp.
  This function blanks out the section of the \a str beginning at \a idx
  and running for \a n characters.
*/
void replaceWithSpace(QString &str, int idx, int n) // Also used in qmlcodemarker.cpp.
{
    QChar *data = str.data() + idx;
    const QChar space(QLatin1Char(' '));
    for (int ii = 0; ii < n; ++ii)
        *data++ = space;
}

/*!
  \struct PragmaInfo

  \brief Container for the result of processing a pragma declaration.

  \list
      \li \c value contains the pragma value (e.g., "library", "singleton").
      \li \c startOffset is the start position in script to replace.
      \li \c length is the length of text to replace.
      \li \c nextToken holds the next token after the pragma.
  \list
*/
struct PragmaInfo {
    QString value;
    qsizetype startOffset;
    qsizetype length;
    int nextToken;
};

/*!
  Processes a ".pragma <value>" declaration (JavaScript syntax).
  Returns pragma information if successfully processed, std::nullopt otherwise.
  Advances the lexer position but does not modify the script content.
*/
static std::optional<PragmaInfo> processDotPragma(QQmlJS::Lexer &lexer, const QStringView script)
{
    static constexpr QLatin1StringView pragma{"pragma"_L1};

    qsizetype startOffset = lexer.tokenOffset();
    qsizetype startLine = lexer.tokenStartLine();

    int token = lexer.lex();

    if (token != QQmlJSGrammar::T_IDENTIFIER || lexer.tokenStartLine() != startLine
        || script.mid(lexer.tokenOffset(), lexer.tokenLength()) != pragma)
        return std::nullopt;

    token = lexer.lex();

    if (token != QQmlJSGrammar::T_IDENTIFIER || lexer.tokenStartLine() != startLine)
        return std::nullopt;

    const QString pragmaValue{script.mid(lexer.tokenOffset(), lexer.tokenLength()).toString()};
    const qsizetype endOffset = lexer.tokenLength() + lexer.tokenOffset();

    token = lexer.lex();
    if (lexer.tokenStartLine() == startLine)
        return std::nullopt;

    if (pragmaValue == "library"_L1) {
        return PragmaInfo{
            pragmaValue,
            startOffset,
            endOffset - startOffset,
            token
        };
    }

    return std::nullopt;
}

/*!
  Copy & paste from src/declarative/qml/qdeclarativescriptparser.cpp,
  then modified to return no values.

  Searches for ".pragma <value>" declarations within \a script.
  Currently supported pragmas are: library
*/
void QmlCodeParser::extractPragmas(QString &script)
{
    QQmlJS::Lexer lexer(nullptr);
    lexer.setCode(script, 0);

    int token = lexer.lex();

    while (true) {
        if (token == QQmlJSGrammar::T_DOT) {
            auto pragmaInfo = processDotPragma(lexer, script);
            if (!pragmaInfo)
                return;

            // Apply the side effects based on helper's feedback
            replaceWithSpace(script, pragmaInfo->startOffset, pragmaInfo->length);
            token = pragmaInfo->nextToken;
            // Continue processing for additional pragmas
        } else {
            return;
        }
    }
}

QT_END_NAMESPACE
