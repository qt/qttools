// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "jscodemarker.h"

#include "atom.h"
#include "node.h"
#include "qmlmarkupvisitor.h"
#include "text.h"

#ifndef QT_NO_DECLARATIVE
#    include <private/qqmljsast_p.h>
#    include <private/qqmljsengine_p.h>
#    include <private/qqmljslexer_p.h>
#    include <private/qqmljsparser_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
  Returns \c true if the \a code is recognized by the parser.
 */
bool JsCodeMarker::recognizeCode(const QString &code)
{
#ifndef QT_NO_DECLARATIVE
    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);
    QQmlJS::Parser parser(&engine);

    const QString &newCode = code;
    lexer.setCode(newCode, 1);

    return parser.parseProgram();
#else
    Q_UNUSED(code);
    return false;
#endif
}

/*!
  Returns \c true if \a ext is any of a list of file extensions
  for the QML language.
 */
bool JsCodeMarker::recognizeExtension(const QString &ext)
{
    return ext == "js";
}

/*!
  Returns \c true if the \a language is recognized. We recognize JavaScript and
  ECMAScript.
 */
bool JsCodeMarker::recognizeLanguage(const QString &language)
{
    return language == "JavaScript" || language == "ECMAScript";
}

/*!
  Returns the type of atom used to represent JavaScript code in the documentation.
*/
Atom::AtomType JsCodeMarker::atomType() const
{
    return Atom::JavaScript;
}

QString JsCodeMarker::markedUpCode(const QString &code, const Node *relative,
                                   const Location &location)
{
    return addMarkUp(code, relative, location);
}

QString JsCodeMarker::addMarkUp(const QString &code, const Node * /* relative */,
                                const Location &location)
{
#ifndef QT_NO_DECLARATIVE
    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);

    QString newCode = code;
    QList<QQmlJS::SourceLocation> pragmas = extractPragmas(newCode);
    lexer.setCode(newCode, 1);

    QQmlJS::Parser parser(&engine);
    QString output;

    if (parser.parseProgram()) {
        QQmlJS::AST::Node *ast = parser.rootNode();
        // Pass the unmodified code to the visitor so that pragmas and other
        // unhandled source text can be output.
        QmlMarkupVisitor visitor(code, pragmas, &engine);
        QQmlJS::AST::Node::accept(ast, &visitor);
        if (visitor.hasError()) {
            location.warning(
                    location.fileName()
                    + QStringLiteral("Unable to analyze JavaScript. The output is incomplete."));
        }
        output = visitor.markedUpCode();
    } else {
        location.warning(
                location.fileName()
                + QStringLiteral("Unable to parse JavaScript: \"%1\" at line %2, column %3")
                          .arg(parser.errorMessage())
                          .arg(parser.errorLineNumber())
                          .arg(parser.errorColumnNumber()));
        output = protect(code);
    }
    return output;
#else
    Q_UNUSED(code);
    location.warning("QtDeclarative not installed; cannot parse QML or JS.");
    return QString();
#endif
}

QT_END_NAMESPACE
