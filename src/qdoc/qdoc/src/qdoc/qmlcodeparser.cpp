// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlcodeparser.h"

#include "node.h"
#include "qmlvisitor.h"
#include "utilities.h"

#include <private/qqmljsast_p.h>

#include <qdebug.h>

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
        COMMAND_VARIABLE, COMMAND_QMLCLASS, COMMAND_QMLTYPE, COMMAND_QMLPROPERTY,
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

    QString newCode = document;
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
  Copy & paste from src/declarative/qml/qdeclarativescriptparser.cpp,
  then modified to return no values.

  Searches for ".pragma <value>" declarations within \a script.
  Currently supported pragmas are: library
*/
void QmlCodeParser::extractPragmas(QString &script)
{
    const QString pragma(QLatin1String("pragma"));

    QQmlJS::Lexer l(nullptr);
    l.setCode(script, 0);

    int token = l.lex();

    while (true) {
        if (token != QQmlJSGrammar::T_DOT)
            return;

        int startOffset = l.tokenOffset();
        int startLine = l.tokenStartLine();

        token = l.lex();

        if (token != QQmlJSGrammar::T_IDENTIFIER || l.tokenStartLine() != startLine
            || script.mid(l.tokenOffset(), l.tokenLength()) != pragma)
            return;

        token = l.lex();

        if (token != QQmlJSGrammar::T_IDENTIFIER || l.tokenStartLine() != startLine)
            return;

        QString pragmaValue = script.mid(l.tokenOffset(), l.tokenLength());
        int endOffset = l.tokenLength() + l.tokenOffset();

        token = l.lex();
        if (l.tokenStartLine() == startLine)
            return;

        if (pragmaValue == QLatin1String("library"))
            replaceWithSpace(script, startOffset, endOffset - startOffset);
        else
            return;
    }
}

QT_END_NAMESPACE
