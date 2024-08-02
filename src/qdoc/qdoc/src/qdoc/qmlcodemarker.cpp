// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlcodemarker.h"

#include <QtCore/qregularexpression.h>

#include "atom.h"
#include "node.h"
#include "qmlmarkupvisitor.h"
#include "text.h"

#include <private/qqmljsast_p.h>
#include <private/qqmljsastfwd_p.h>
#include <private/qqmljsengine_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>

QT_BEGIN_NAMESPACE

/*!
  Returns \c true if the \a code is recognized by the parser.
 */
bool QmlCodeMarker::recognizeCode(const QString &code)
{
    // Naive pre-check; starts with an import statement or 'CamelCase {'
    static const QRegularExpression regExp(QStringLiteral("^\\s*(import |([A-Z][a-z0-9]*)+\\s?{)"));
    if (!regExp.match(code).hasMatch())
        return false;

    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);
    QQmlJS::Parser parser(&engine);

    QString newCode = code;
    extractPragmas(newCode);
    lexer.setCode(newCode, 1);

    return parser.parse();
}

/*!
  Returns \c true if \a ext is any of a list of file extensions
  for the QML language.
 */
bool QmlCodeMarker::recognizeExtension(const QString &ext)
{
    return ext == "qml";
}

/*!
  Returns \c true if the \a language is recognized. Only "QML" is
  recognized by this marker.
 */
bool QmlCodeMarker::recognizeLanguage(const QString &language)
{
    return language == "QML";
}

/*!
  Returns the type of atom used to represent QML code in the documentation.
*/
Atom::AtomType QmlCodeMarker::atomType() const
{
    return Atom::Qml;
}

QString QmlCodeMarker::markedUpCode(const QString &code, const Node *relative,
                                    const Location &location)
{
    return addMarkUp(code, relative, location);
}

/*!
  Constructs and returns the marked up name for the \a node.
  If the node is any kind of QML function (a method,
  signal, or handler), "()" is appended to the marked up name.
 */
QString QmlCodeMarker::markedUpName(const Node *node)
{
    QString name = linkTag(node, taggedNode(node));
    if (node->isFunction())
        name += "()";
    return name;
}

QString QmlCodeMarker::addMarkUp(const QString &code, const Node * /* relative */,
                                 const Location &location)
{
    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);

    QString newCode = code;
    QList<QQmlJS::SourceLocation> pragmas = extractPragmas(newCode);
    lexer.setCode(newCode, 1);

    QQmlJS::Parser parser(&engine);
    QString output;

    if (parser.parse()) {
        QQmlJS::AST::UiProgram *ast = parser.ast();
        // Pass the unmodified code to the visitor so that pragmas and other
        // unhandled source text can be output.
        QmlMarkupVisitor visitor(code, pragmas, &engine);
        QQmlJS::AST::Node::accept(ast, &visitor);
        if (visitor.hasError()) {
            location.warning(
                    location.fileName()
                    + QStringLiteral("Unable to analyze QML snippet. The output is incomplete."));
        }
        output = visitor.markedUpCode();
    } else {
        location.warning(QStringLiteral("Unable to parse QML snippet: \"%1\" at line %2, column %3")
                                 .arg(parser.errorMessage())
                                 .arg(parser.errorLineNumber())
                                 .arg(parser.errorColumnNumber()));
        output = protect(code);
    }

    return output;
}

/*
  Copied and pasted from
  src/declarative/qml/qqmlscriptparser.cpp.
*/
void replaceWithSpace(QString &str, int idx, int n); // qmlcodeparser.cpp

/*
  Copied and pasted from
  src/declarative/qml/qqmlscriptparser.cpp then modified to
  return a list of removed pragmas.

  Searches for ".pragma <value>" or ".import <stuff>" declarations
  in \a script. Currently supported pragmas are: library
*/
QList<QQmlJS::SourceLocation> QmlCodeMarker::extractPragmas(QString &script)
{
    QList<QQmlJS::SourceLocation> removed;

    QQmlJS::Lexer l(nullptr);
    l.setCode(script, 0);

    int token = l.lex();

    while (true) {
        if (token != QQmlJSGrammar::T_DOT)
            break;

        int startOffset = l.tokenOffset();
        int startLine = l.tokenStartLine();
        int startColumn = l.tokenStartColumn();

        token = l.lex();

        if (token != QQmlJSGrammar::T_PRAGMA && token != QQmlJSGrammar::T_IMPORT)
            break;
        int endOffset = 0;
        while (startLine == l.tokenStartLine()) {
            endOffset = l.tokenLength() + l.tokenOffset();
            token = l.lex();
        }
        replaceWithSpace(script, startOffset, endOffset - startOffset);
        removed.append(QQmlJS::SourceLocation(startOffset, endOffset - startOffset, startLine,
                                                   startColumn));
    }
    return removed;
}

QT_END_NAMESPACE
