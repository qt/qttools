/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
  qmlcodemarker.cpp
*/

#include "qmlcodemarker.h"

#include "atom.h"
#include "node.h"
#include "qmlmarkupvisitor.h"
#include "text.h"
#include "tree.h"
#include "generator.h"

#ifndef QT_NO_DECLARATIVE
#    include <private/qqmljsast_p.h>
#    include <private/qqmljsastfwd_p.h>
#    include <private/qqmljsengine_p.h>
#    include <private/qqmljslexer_p.h>
#    include <private/qqmljsparser_p.h>
#endif

QT_BEGIN_NAMESPACE

QmlCodeMarker::QmlCodeMarker() {}

QmlCodeMarker::~QmlCodeMarker() {}

/*!
  Returns \c true if the \a code is recognized by the parser.
 */
bool QmlCodeMarker::recognizeCode(const QString &code)
{
#ifndef QT_NO_DECLARATIVE
    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);
    QQmlJS::Parser parser(&engine);

    QString newCode = code;
    extractPragmas(newCode);
    lexer.setCode(newCode, 1);

    return parser.parse();
#else
    return false;
#endif
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
  If the node is any kind of QML or JS function (a method,
  signal, or handler), "()" is appended to the marked up name.
 */
QString QmlCodeMarker::markedUpName(const Node *node)
{
    QString name = linkTag(node, taggedNode(node));
    if (node->isFunction())
        name += "()";
    return name;
}

QString QmlCodeMarker::markedUpFullName(const Node *node, const Node *relative)
{
    if (node->name().isEmpty()) {
        return "global";
    } else {
        QString fullName;
        for (;;) {
            fullName.prepend(markedUpName(node));
            if (node->parent() == relative || node->parent()->name().isEmpty())
                break;
            fullName.prepend("<@op>::</@op>");
            node = node->parent();
        }
        return fullName;
    }
}

QString QmlCodeMarker::markedUpIncludes(const QStringList &includes)
{
    QString code;

    for (const auto &include : includes)
        code += "import " + include + QLatin1Char('\n');

    Location location;
    return addMarkUp(code, nullptr, location);
}

QString QmlCodeMarker::functionBeginRegExp(const QString &funcName)
{
    return QLatin1Char('^') + QRegExp::escape("function " + funcName) + QLatin1Char('$');
}

QString QmlCodeMarker::functionEndRegExp(const QString & /* funcName */)
{
    return "^\\}$";
}

QString QmlCodeMarker::addMarkUp(const QString &code, const Node * /* relative */,
                                 const Location &location)
{
#ifndef QT_NO_DECLARATIVE
    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);

    QString newCode = code;
    QVector<QQmlJS::SourceLocation> pragmas = extractPragmas(newCode);
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
            location.warning(location.fileName()
                             + tr("Unable to analyze QML snippet. The output is incomplete."));
        }
        output = visitor.markedUpCode();
    } else {
        location.warning(tr("Unable to parse QML snippet: \"%1\" at line %2, column %3")
                                 .arg(parser.errorMessage())
                                 .arg(parser.errorLineNumber())
                                 .arg(parser.errorColumnNumber()));
        output = protect(code);
    }

    return output;
#else
    location.warning("QtDeclarative not installed; cannot parse QML or JS.");
    return QString();
#endif
}

#ifndef QT_NO_DECLARATIVE
/*
  Copied and pasted from
  src/declarative/qml/qqmlscriptparser.cpp.
*/
static void replaceWithSpace(QString &str, int idx, int n)
{
    QChar *data = str.data() + idx;
    const QChar space(QLatin1Char(' '));
    for (int ii = 0; ii < n; ++ii)
        *data++ = space;
}

/*
  Copied and pasted from
  src/declarative/qml/qqmlscriptparser.cpp then modified to
  return a list of removed pragmas.

  Searches for ".pragma <value>" or ".import <stuff>" declarations
  in \a script. Currently supported pragmas are: library
*/
QVector<QQmlJS::SourceLocation> QmlCodeMarker::extractPragmas(QString &script)
{
    const QString pragma(QLatin1String("pragma"));
    const QString library(QLatin1String("library"));
    QVector<QQmlJS::SourceLocation> removed;

    QQmlJS::Lexer l(nullptr);
    l.setCode(script, 0);

    int token = l.lex();

    while (true) {
        if (token != QQmlJSGrammar::T_DOT)
            return removed;

        int startOffset = l.tokenOffset();
        int startLine = l.tokenStartLine();
        int startColumn = l.tokenStartColumn();

        token = l.lex();

        if (token != QQmlJSGrammar::T_PRAGMA && token != QQmlJSGrammar::T_IMPORT)
            return removed;
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
#endif

QT_END_NAMESPACE
