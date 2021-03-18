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
  cppcodemarker.cpp
*/

#include "cppcodemarker.h"

#include "generator.h"
#include "text.h"
#include "tree.h"

#include <QtCore/qdebug.h>
#include <QtCore/qregexp.h>

#include <ctype.h>

QT_BEGIN_NAMESPACE

/*!
  The constructor does nothing.
 */
CppCodeMarker::CppCodeMarker()
{
    // nothing.
}

/*!
  The destructor does nothing.
 */
CppCodeMarker::~CppCodeMarker()
{
    // nothing.
}

/*!
  Returns \c true.
 */
bool CppCodeMarker::recognizeCode(const QString & /* code */)
{
    return true;
}

/*!
  Returns \c true if \a ext is any of a list of file extensions
  for the C++ language.
 */
bool CppCodeMarker::recognizeExtension(const QString &extension)
{
    QByteArray ext = extension.toLatin1();
    return ext == "c" || ext == "c++" || ext == "qdoc" || ext == "qtt" || ext == "qtx"
            || ext == "cc" || ext == "cpp" || ext == "cxx" || ext == "ch" || ext == "h"
            || ext == "h++" || ext == "hh" || ext == "hpp" || ext == "hxx";
}

/*!
  Returns \c true if \a lang is either "C" or "Cpp".
 */
bool CppCodeMarker::recognizeLanguage(const QString &lang)
{
    return lang == QLatin1String("C") || lang == QLatin1String("Cpp");
}

/*!
  Returns the type of atom used to represent C++ code in the documentation.
*/
Atom::AtomType CppCodeMarker::atomType() const
{
    return Atom::Code;
}

QString CppCodeMarker::markedUpCode(const QString &code, const Node *relative,
                                    const Location &location)
{
    return addMarkUp(code, relative, location);
}

QString CppCodeMarker::markedUpSynopsis(const Node *node, const Node * /* relative */,
                                        Section::Style style)
{
    const int MaxEnumValues = 6;
    const FunctionNode *func;
    const PropertyNode *property;
    const VariableNode *variable;
    const EnumNode *enume;
    const TypedefNode *typedeff;
    QString synopsis;
    QString extra;
    QString name;

    name = taggedNode(node);
    if (style != Section::Details)
        name = linkTag(node, name);
    name = "<@name>" + name + "</@name>";

    if (style == Section::Details) {
        if (!node->isRelatedNonmember() && !node->isProxyNode() && !node->parent()->name().isEmpty()
            && !node->parent()->isHeader() && !node->isProperty() && !node->isQmlNode()
            && !node->isJsNode()) {
            name.prepend(taggedNode(node->parent()) + "::");
        }
    }

    switch (node->nodeType()) {
    case Node::Namespace:
    case Node::Class:
    case Node::Struct:
    case Node::Union:
        synopsis = Node::nodeTypeString(node->nodeType());
        synopsis += QLatin1Char(' ') + name;
        break;
    case Node::Function:
        func = (const FunctionNode *)node;
        if (style == Section::Details) {
            QString templateDecl = node->templateDecl();
            if (!templateDecl.isEmpty())
                synopsis = templateDecl + QLatin1Char(' ');
        }
        if (style != Section::AllMembers && !func->returnType().isEmpty())
            synopsis += typified(func->returnType(), true);
        synopsis += name;
        if (!func->isMacroWithoutParams()) {
            synopsis += QLatin1Char('(');
            if (!func->parameters().isEmpty()) {
                const Parameters &parameters = func->parameters();
                for (int i = 0; i < parameters.count(); ++i) {
                    if (i > 0)
                        synopsis += ", ";
                    QString name = parameters.at(i).name();
                    QString type = parameters.at(i).type();
                    QString value = parameters.at(i).defaultValue();
                    QString paramName;
                    if (!name.isEmpty()) {
                        synopsis += typified(type, true);
                        paramName = name;
                    } else {
                        paramName = type;
                    }
                    if (style != Section::AllMembers || name.isEmpty())
                        synopsis += "<@param>" + protect(paramName) + "</@param>";
                    if (style != Section::AllMembers && !value.isEmpty())
                        synopsis += " = " + protect(value);
                }
            }
            synopsis += QLatin1Char(')');
        }
        if (func->isConst())
            synopsis += " const";

        if (style == Section::Summary || style == Section::Accessors) {
            if (!func->isNonvirtual())
                synopsis.prepend("virtual ");
            if (func->isFinal())
                synopsis.append(" final");
            if (func->isOverride())
                synopsis.append(" override");
            if (func->isPureVirtual())
                synopsis.append(" = 0");
            if (func->isRef())
                synopsis.append(" &");
            else if (func->isRefRef())
                synopsis.append(" &&");
        } else if (style == Section::AllMembers) {
            if (!func->returnType().isEmpty() && func->returnType() != "void")
                synopsis += " : " + typified(func->returnType());
        } else {
            if (func->isRef())
                synopsis.append(" &");
            else if (func->isRefRef())
                synopsis.append(" &&");
            QStringList bracketed;
            if (func->isStatic()) {
                bracketed += "static";
            } else if (!func->isNonvirtual()) {
                if (func->isFinal())
                    bracketed += "final";
                if (func->isOverride())
                    bracketed += "override";
                if (func->isPureVirtual())
                    bracketed += "pure";
                bracketed += "virtual";
            }

            if (func->access() == Node::Protected)
                bracketed += "protected";
            else if (func->access() == Node::Private)
                bracketed += "private";

            if (func->isSignal())
                bracketed += "signal";
            else if (func->isSlot())
                bracketed += "slot";
            if (!bracketed.isEmpty())
                extra += QLatin1Char('[') + bracketed.join(' ') + QStringLiteral("] ");
        }
        break;
    case Node::Enum:
        enume = static_cast<const EnumNode *>(node);
        synopsis = "enum ";
        if (enume->isScoped())
            synopsis += "class ";
        synopsis += name;
        if (style == Section::Summary) {
            synopsis += " { ";

            QStringList documentedItems = enume->doc().enumItemNames();
            if (documentedItems.isEmpty()) {
                const auto &enumItems = enume->items();
                for (const auto &item : enumItems)
                    documentedItems << item.name();
            }
            const QStringList omitItems = enume->doc().omitEnumItemNames();
            for (const auto &item : omitItems)
                documentedItems.removeAll(item);

            if (documentedItems.size() > MaxEnumValues) {
                // Take the last element and keep it safe, then elide the surplus.
                const QString last = documentedItems.last();
                documentedItems = documentedItems.mid(0, MaxEnumValues - 1);
                documentedItems += "&hellip;";
                documentedItems += last;
            }
            synopsis += documentedItems.join(QLatin1String(", "));

            if (!documentedItems.isEmpty())
                synopsis += QLatin1Char(' ');
            synopsis += QLatin1Char('}');
        }
        break;
    case Node::TypeAlias:
        if (style == Section::Summary)
            synopsis = "(alias) ";
        else if (style == Section::Details) {
            extra = QStringLiteral("[alias] ");
            QString templateDecl = node->templateDecl();
            if (!templateDecl.isEmpty())
                synopsis = templateDecl + QLatin1Char(' ');
        }
        synopsis += name;
        break;
    case Node::Typedef:
        typedeff = static_cast<const TypedefNode *>(node);
        if (typedeff->associatedEnum()) {
            synopsis = "flags " + name;
        } else {
            synopsis = "typedef " + name;
        }
        break;
    case Node::Property:
        property = static_cast<const PropertyNode *>(node);
        synopsis = name + " : " + typified(property->qualifiedDataType());
        break;
    case Node::Variable:
        variable = static_cast<const VariableNode *>(node);
        if (style == Section::AllMembers) {
            synopsis = name + " : " + typified(variable->dataType());
        } else {
            synopsis = typified(variable->leftType(), true) + name + protect(variable->rightType());
        }
        break;
    default:
        synopsis = name;
    }

    if (style == Section::Summary) {
        if (node->isPreliminary())
            extra += "(preliminary) ";
        else if (node->isDeprecated())
            extra += "(deprecated) ";
        else if (node->isObsolete())
            extra += "(obsolete) ";
    }

    if (!extra.isEmpty()) {
        extra.prepend("<@extra>");
        extra.append("</@extra>");
    }
    return extra + synopsis;
}

/*!
 */
QString CppCodeMarker::markedUpQmlItem(const Node *node, bool summary)
{
    QString name = taggedQmlNode(node);
    if (summary) {
        name = linkTag(node, name);
    } else if (node->isQmlProperty() || node->isJsProperty()) {
        const QmlPropertyNode *pn = static_cast<const QmlPropertyNode *>(node);
        if (pn->isAttached())
            name.prepend(pn->element() + QLatin1Char('.'));
    }
    name = "<@name>" + name + "</@name>";
    QString synopsis;
    if (node->isQmlProperty() || node->isJsProperty()) {
        const QmlPropertyNode *pn = static_cast<const QmlPropertyNode *>(node);
        synopsis = name + " : " + typified(pn->dataType());
    } else if (node->isFunction(Node::QML) || node->isFunction(Node::JS)) {
        const FunctionNode *func = static_cast<const FunctionNode *>(node);
        if (!func->returnType().isEmpty())
            synopsis = typified(func->returnType(), true) + name;
        else
            synopsis = name;
        synopsis += QLatin1Char('(');
        if (!func->parameters().isEmpty()) {
            const Parameters &parameters = func->parameters();
            for (int i = 0; i < parameters.count(); ++i) {
                if (i > 0)
                    synopsis += ", ";
                QString name = parameters.at(i).name();
                QString type = parameters.at(i).type();
                QString paramName;
                if (!name.isEmpty()) {
                    synopsis += typified(type, true);
                    paramName = name;
                } else {
                    paramName = type;
                }
                synopsis += "<@param>" + protect(paramName) + "</@param>";
            }
        }
        synopsis += QLatin1Char(')');
    } else {
        synopsis = name;
    }

    QString extra;
    if (summary) {
        if (node->isPreliminary())
            extra += " (preliminary)";
        else if (node->isDeprecated())
            extra += " (deprecated)";
        else if (node->isObsolete())
            extra += " (obsolete)";
    }

    if (!extra.isEmpty()) {
        extra.prepend("<@extra>");
        extra.append("</@extra>");
    }
    return synopsis + extra;
}

QString CppCodeMarker::markedUpName(const Node *node)
{
    QString name = linkTag(node, taggedNode(node));
    if (node->isFunction() && !node->isMacro())
        name += "()";
    return name;
}

QString CppCodeMarker::markedUpFullName(const Node *node, const Node *relative)
{
    if (node->name().isEmpty())
        return "global";

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

QString CppCodeMarker::markedUpEnumValue(const QString &enumValue, const Node *relative)
{
    if (!relative->isEnumType())
        return enumValue;

    const Node *node = relative->parent();
    QStringList parts;
    while (!node->isHeader() && node->parent()) {
        parts.prepend(markedUpName(node));
        if (node->parent() == relative || node->parent()->name().isEmpty())
            break;
        node = node->parent();
    }
    if (static_cast<const EnumNode *>(relative)->isScoped())
        parts.append(relative->name());

    parts.append(enumValue);
    return parts.join(QLatin1String("<@op>::</@op>"));
}

QString CppCodeMarker::markedUpIncludes(const QStringList &includes)
{
    QString code;

    for (const auto &include : includes)
        code += "<@preprocessor>#include &lt;<@headerfile>" + include
                + "</@headerfile>&gt;</@preprocessor>\n";
    return code;
}

QString CppCodeMarker::functionBeginRegExp(const QString &funcName)
{
    return QLatin1Char('^') + QRegExp::escape(funcName) + QLatin1Char('$');
}

QString CppCodeMarker::functionEndRegExp(const QString & /* funcName */)
{
    return "^\\}$";
}

/*
    @char
    @class
    @comment
    @function
    @keyword
    @number
    @op
    @preprocessor
    @string
    @type
*/

QString CppCodeMarker::addMarkUp(const QString &in, const Node * /* relative */,
                                 const Location & /* location */)
{
    static QSet<QString> types;
    static QSet<QString> keywords;

    if (types.isEmpty()) {
        // initialize statics
        Q_ASSERT(keywords.isEmpty());
        static const QString typeTable[] = {
            QLatin1String("bool"),       QLatin1String("char"),    QLatin1String("double"),
            QLatin1String("float"),      QLatin1String("int"),     QLatin1String("long"),
            QLatin1String("short"),      QLatin1String("signed"),  QLatin1String("unsigned"),
            QLatin1String("uint"),       QLatin1String("ulong"),   QLatin1String("ushort"),
            QLatin1String("uchar"),      QLatin1String("void"),    QLatin1String("qlonglong"),
            QLatin1String("qulonglong"), QLatin1String("qint"),    QLatin1String("qint8"),
            QLatin1String("qint16"),     QLatin1String("qint32"),  QLatin1String("qint64"),
            QLatin1String("quint"),      QLatin1String("quint8"),  QLatin1String("quint16"),
            QLatin1String("quint32"),    QLatin1String("quint64"), QLatin1String("qreal"),
            QLatin1String("cond")
        };

        static const QString keywordTable[] = {
            QLatin1String("and"), QLatin1String("and_eq"), QLatin1String("asm"),
            QLatin1String("auto"), QLatin1String("bitand"), QLatin1String("bitor"),
            QLatin1String("break"), QLatin1String("case"), QLatin1String("catch"),
            QLatin1String("class"), QLatin1String("compl"), QLatin1String("const"),
            QLatin1String("const_cast"), QLatin1String("continue"), QLatin1String("default"),
            QLatin1String("delete"), QLatin1String("do"), QLatin1String("dynamic_cast"),
            QLatin1String("else"), QLatin1String("enum"), QLatin1String("explicit"),
            QLatin1String("export"), QLatin1String("extern"), QLatin1String("false"),
            QLatin1String("for"), QLatin1String("friend"), QLatin1String("goto"),
            QLatin1String("if"), QLatin1String("include"), QLatin1String("inline"),
            QLatin1String("monitor"), QLatin1String("mutable"), QLatin1String("namespace"),
            QLatin1String("new"), QLatin1String("not"), QLatin1String("not_eq"),
            QLatin1String("operator"), QLatin1String("or"), QLatin1String("or_eq"),
            QLatin1String("private"), QLatin1String("protected"), QLatin1String("public"),
            QLatin1String("register"), QLatin1String("reinterpret_cast"), QLatin1String("return"),
            QLatin1String("sizeof"), QLatin1String("static"), QLatin1String("static_cast"),
            QLatin1String("struct"), QLatin1String("switch"), QLatin1String("template"),
            QLatin1String("this"), QLatin1String("throw"), QLatin1String("true"),
            QLatin1String("try"), QLatin1String("typedef"), QLatin1String("typeid"),
            QLatin1String("typename"), QLatin1String("union"), QLatin1String("using"),
            QLatin1String("virtual"), QLatin1String("volatile"), QLatin1String("wchar_t"),
            QLatin1String("while"), QLatin1String("xor"), QLatin1String("xor_eq"),
            QLatin1String("synchronized"),
            // Qt specific
            QLatin1String("signals"), QLatin1String("slots"), QLatin1String("emit")
        };

        types.reserve(sizeof(typeTable) / sizeof(QString));
        for (int j = sizeof(typeTable) / sizeof(QString) - 1; j; --j)
            types.insert(typeTable[j]);

        keywords.reserve(sizeof(keywordTable) / sizeof(QString));
        for (int j = sizeof(keywordTable) / sizeof(QString) - 1; j; --j)
            keywords.insert(keywordTable[j]);
    }
#define readChar() ch = (i < code.length()) ? code[i++].cell() : EOF

    QString code = in;
    QString out;
    QStringRef text;
    int braceDepth = 0;
    int parenDepth = 0;
    int i = 0;
    int start = 0;
    int finish = 0;
    QChar ch;
    QRegExp classRegExp("Qt?(?:[A-Z3]+[a-z][A-Za-z]*|t)");
    QRegExp functionRegExp("q([A-Z][a-z]+)+");
    QRegExp findFunctionRegExp(QStringLiteral("^\\s*\\("));

    readChar();

    while (ch != QChar(EOF)) {
        QString tag;
        bool target = false;

        if (ch.isLetter() || ch == '_') {
            QString ident;
            do {
                ident += ch;
                finish = i;
                readChar();
            } while (ch.isLetterOrNumber() || ch == '_');

            if (classRegExp.exactMatch(ident)) {
                tag = QStringLiteral("type");
            } else if (functionRegExp.exactMatch(ident)) {
                tag = QStringLiteral("func");
                target = true;
            } else if (types.contains(ident)) {
                tag = QStringLiteral("type");
            } else if (keywords.contains(ident)) {
                tag = QStringLiteral("keyword");
            } else if (braceDepth == 0 && parenDepth == 0) {
                if (code.indexOf(findFunctionRegExp, i - 1) == i - 1)
                    tag = QStringLiteral("func");
                target = true;
            }
        } else if (ch.isDigit()) {
            do {
                finish = i;
                readChar();
            } while (ch.isLetterOrNumber() || ch == '.');
            tag = QStringLiteral("number");
        } else {
            switch (ch.unicode()) {
            case '+':
            case '-':
            case '!':
            case '%':
            case '^':
            case '&':
            case '*':
            case ',':
            case '.':
            case '<':
            case '=':
            case '>':
            case '?':
            case '[':
            case ']':
            case '|':
            case '~':
                finish = i;
                readChar();
                tag = QStringLiteral("op");
                break;
            case '"':
                finish = i;
                readChar();

                while (ch != QChar(EOF) && ch != '"') {
                    if (ch == '\\')
                        readChar();
                    readChar();
                }
                finish = i;
                readChar();
                tag = QStringLiteral("string");
                break;
            case '#':
                finish = i;
                readChar();
                while (ch != QChar(EOF) && ch != '\n') {
                    if (ch == '\\')
                        readChar();
                    finish = i;
                    readChar();
                }
                tag = QStringLiteral("preprocessor");
                break;
            case '\'':
                finish = i;
                readChar();

                while (ch != QChar(EOF) && ch != '\'') {
                    if (ch == '\\')
                        readChar();
                    readChar();
                }
                finish = i;
                readChar();
                tag = QStringLiteral("char");
                break;
            case '(':
                finish = i;
                readChar();
                ++parenDepth;
                break;
            case ')':
                finish = i;
                readChar();
                --parenDepth;
                break;
            case ':':
                finish = i;
                readChar();
                if (ch == ':') {
                    finish = i;
                    readChar();
                    tag = QStringLiteral("op");
                }
                break;
            case '/':
                finish = i;
                readChar();
                if (ch == '/') {
                    do {
                        finish = i;
                        readChar();
                    } while (ch != QChar(EOF) && ch != '\n');
                    tag = QStringLiteral("comment");
                } else if (ch == '*') {
                    bool metAster = false;
                    bool metAsterSlash = false;

                    finish = i;
                    readChar();

                    while (!metAsterSlash) {
                        if (ch == QChar(EOF))
                            break;

                        if (ch == '*')
                            metAster = true;
                        else if (metAster && ch == '/')
                            metAsterSlash = true;
                        else
                            metAster = false;
                        finish = i;
                        readChar();
                    }
                    tag = QStringLiteral("comment");
                } else {
                    tag = QStringLiteral("op");
                }
                break;
            case '{':
                finish = i;
                readChar();
                braceDepth++;
                break;
            case '}':
                finish = i;
                readChar();
                braceDepth--;
                break;
            default:
                finish = i;
                readChar();
            }
        }

        text = code.midRef(start, finish - start);
        start = finish;

        if (!tag.isEmpty()) {
            out += QStringLiteral("<@");
            out += tag;
            if (target) {
                out += QStringLiteral(" target=\"");
                out += text;
                out += QStringLiteral("()\"");
            }
            out += QStringLiteral(">");
        }

        appendProtectedString(&out, text);

        if (!tag.isEmpty()) {
            out += QStringLiteral("</@");
            out += tag;
            out += QStringLiteral(">");
        }
    }

    if (start < code.length()) {
        appendProtectedString(&out, code.midRef(start));
    }

    return out;
}

QT_END_NAMESPACE
