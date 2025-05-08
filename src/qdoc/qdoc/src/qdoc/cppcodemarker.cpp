// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cppcodemarker.h"

#include "access.h"
#include "enumnode.h"
#include "functionnode.h"
#include "genustypes.h"
#include "namespacenode.h"
#include "propertynode.h"
#include "qmlpropertynode.h"
#include "text.h"
#include "tree.h"
#include "typedefnode.h"
#include "variablenode.h"

#include <QtCore/qdebug.h>
#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

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
    const VariableNode *variable;
    const EnumNode *enume;
    QString synopsis;
    QString name;

    name = taggedNode(node);
    if (style != Section::Details)
        name = linkTag(node, name);
    name = "<@name>" + name + "</@name>";

    if (style == Section::Details) {
        if (!node->isRelatedNonmember() && !node->isProxyNode() && !node->parent()->name().isEmpty()
            && !node->parent()->isHeader() && !node->isProperty() && !node->isQmlNode()) {
            name.prepend(taggedNode(node->parent()) + "::");
        }
    }

    switch (node->nodeType()) {
    case NodeType::Namespace:
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        synopsis = Node::nodeTypeString(node->nodeType());
        synopsis += QLatin1Char(' ') + name;
        break;
    case NodeType::Function:
        func = (const FunctionNode *)node;
        if (style == Section::Details) {
            auto templateDecl = node->templateDecl();
            if (templateDecl)
                synopsis = protect((*templateDecl).to_qstring()) + QLatin1Char(' ');
        }
        if (style != Section::AllMembers && !func->returnType().isEmpty())
            synopsis += typified(func->returnTypeString(), true);
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
                    bool trailingSpace = style != Section::AllMembers && !name.isEmpty();
                    synopsis += typified(type, trailingSpace);
                    if (style != Section::AllMembers && !name.isEmpty())
                        synopsis += "<@param>" + protect(name) + "</@param>";
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
                synopsis += " : " + typified(func->returnTypeString());
        } else {
            if (func->isRef())
                synopsis.append(" &");
            else if (func->isRefRef())
                synopsis.append(" &&");
        }
        break;
    case NodeType::Enum:
        enume = static_cast<const EnumNode *>(node);
        synopsis = "enum";
        if (enume->isScoped())
            synopsis += " class";
        if (!enume->isAnonymous())
            synopsis += " %1"_L1.arg(name);
        else if (style != Section::Details)
            synopsis = linkTag(node, synopsis); // Unnamed enum: Make `enum` a link to details
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
    case NodeType::TypeAlias:
        if (style == Section::Details) {
            auto templateDecl = node->templateDecl();
            if (templateDecl)
                synopsis += protect((*templateDecl).to_qstring()) + QLatin1Char(' ');
        }
        synopsis += name;
        break;
    case NodeType::Typedef:
        if (static_cast<const TypedefNode *>(node)->associatedEnum())
            synopsis = "flags ";
        synopsis += name;
        break;
    case NodeType::Property: {
        auto property = static_cast<const PropertyNode *>(node);
        synopsis = name + " : " + typified(property->qualifiedDataType());
        break;
    }
    case NodeType::QmlProperty: {
        auto property = static_cast<const QmlPropertyNode *>(node);
        synopsis = name + " : " + typified(property->dataType());
        break;
    }
    case NodeType::Variable:
        variable = static_cast<const VariableNode *>(node);
        if (style == Section::AllMembers) {
            synopsis = name + " : " + typified(variable->dataType());
        } else {
            synopsis = typified(variable->leftType(), true) + name + protect(variable->rightType());
        }
        break;
    default:
        synopsis = std::move(name);
    }

    QString extra = CodeMarker::extraSynopsis(node, style);
    if (!extra.isEmpty()) {
        extra.prepend(u"<@extra>"_s);
        extra.append(u"</@extra> "_s);
    }

    return extra + synopsis;
}

/*!
 */
QString CppCodeMarker::markedUpQmlItem(const Node *node, bool summary)
{
    QString name = taggedQmlNode(node);
    QString synopsis;

    if (summary) {
        name = linkTag(node, name);
    } else if (node->isQmlProperty()) {
        const auto *pn = static_cast<const QmlPropertyNode *>(node);
        if (pn->isAttached())
            name.prepend(pn->element() + QLatin1Char('.'));
    }
    name = "<@name>" + name + "</@name>";
    if (node->isQmlProperty()) {
        const auto *pn = static_cast<const QmlPropertyNode *>(node);
        synopsis = name + " : " + typified(pn->dataType());
    } else if (node->isFunction(Genus::QML)) {
        const auto *func = static_cast<const FunctionNode *>(node);
        if (!func->returnType().isEmpty())
            synopsis = typified(func->returnTypeString(), true) + name;
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
                    paramName = std::move(name);
                } else {
                    paramName = std::move(type);
                }
                synopsis += "<@param>" + protect(paramName) + "</@param>";
            }
        }
        synopsis += QLatin1Char(')');
    } else {
        synopsis = std::move(name);
    }

    QString extra = CodeMarker::extraSynopsis(node, summary ? Section::Summary : Section::Details);
    if (!extra.isEmpty()) {
        extra.prepend(u" <@extra>"_s);
        extra.append(u"</@extra>"_s);
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

QString CppCodeMarker::markedUpEnumValue(const QString &enumValue, const Node *relative)
{
    const auto *node = relative->parent();

    const NativeEnum *nativeEnum{nullptr};
    if (auto *ne_if = dynamic_cast<const NativeEnumInterface *>(relative))
        nativeEnum = ne_if->nativeEnum();

    if (nativeEnum && nativeEnum->enumNode()
            && !enumValue.startsWith("%1."_L1.arg(nativeEnum->prefix())))
        return "%1<@op>.</@op>%2"_L1.arg(nativeEnum->prefix(), enumValue);

    if (!relative->isEnumType()) {
        return enumValue;
    }

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
    const auto &delim = (relative->genus() == Genus::QML) ? "."_L1 : "::"_L1;
    return parts.join("<@op>%1</@op>"_L1.arg(delim));
}

QString CppCodeMarker::addMarkUp(const QString &in, const Node * /* relative */,
                                 const Location & /* location */)
{
    static QSet<QString> types{
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

    static QSet<QString> keywords{
        QLatin1String("and"), QLatin1String("and_eq"), QLatin1String("asm"), QLatin1String("auto"),
        QLatin1String("bitand"), QLatin1String("bitor"), QLatin1String("break"),
        QLatin1String("case"), QLatin1String("catch"), QLatin1String("class"),
        QLatin1String("compl"), QLatin1String("const"), QLatin1String("const_cast"),
        QLatin1String("continue"), QLatin1String("default"), QLatin1String("delete"),
        QLatin1String("do"), QLatin1String("dynamic_cast"), QLatin1String("else"),
        QLatin1String("enum"), QLatin1String("explicit"), QLatin1String("export"),
        QLatin1String("extern"), QLatin1String("false"), QLatin1String("for"),
        QLatin1String("friend"), QLatin1String("goto"), QLatin1String("if"),
        QLatin1String("include"), QLatin1String("inline"), QLatin1String("monitor"),
        QLatin1String("mutable"), QLatin1String("namespace"), QLatin1String("new"),
        QLatin1String("not"), QLatin1String("not_eq"), QLatin1String("operator"),
        QLatin1String("or"), QLatin1String("or_eq"), QLatin1String("private"),
        QLatin1String("protected"), QLatin1String("public"), QLatin1String("register"),
        QLatin1String("reinterpret_cast"), QLatin1String("return"), QLatin1String("sizeof"),
        QLatin1String("static"), QLatin1String("static_cast"), QLatin1String("struct"),
        QLatin1String("switch"), QLatin1String("template"), QLatin1String("this"),
        QLatin1String("throw"), QLatin1String("true"), QLatin1String("try"),
        QLatin1String("typedef"), QLatin1String("typeid"), QLatin1String("typename"),
        QLatin1String("union"), QLatin1String("using"), QLatin1String("virtual"),
        QLatin1String("volatile"), QLatin1String("wchar_t"), QLatin1String("while"),
        QLatin1String("xor"), QLatin1String("xor_eq"), QLatin1String("synchronized"),
        // Qt specific
        QLatin1String("signals"), QLatin1String("slots"), QLatin1String("emit")
    };

    QString code = in;
    QString out;
    QStringView text;
    int braceDepth = 0;
    int parenDepth = 0;
    int i = 0;
    int start = 0;
    int finish = 0;
    QChar ch;
    static const QRegularExpression classRegExp(QRegularExpression::anchoredPattern("Qt?(?:[A-Z3]+[a-z][A-Za-z]*|t)"));
    static const QRegularExpression functionRegExp(QRegularExpression::anchoredPattern("q([A-Z][a-z]+)+"));
    static const QRegularExpression findFunctionRegExp(QStringLiteral("^\\s*\\("));
    bool atEOF = false;

    auto readChar = [&]() {
         if (i < code.size())
            ch = code[i++];
         else
            atEOF = true;
    };

    readChar();
    while (!atEOF) {
        QString tag;
        bool target = false;

        if (ch.isLetter() || ch == '_') {
            QString ident;
            do {
                ident += ch;
                finish = i;
                readChar();
            } while (!atEOF && (ch.isLetterOrNumber() || ch == '_'));

            if (classRegExp.match(ident).hasMatch()) {
                tag = QStringLiteral("type");
            } else if (functionRegExp.match(ident).hasMatch()) {
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
            } while (!atEOF && (ch.isLetterOrNumber() || ch == '.' || ch == '\''));
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

                while (!atEOF && ch != '"') {
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
                while (!atEOF && ch != '\n') {
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

                while (!atEOF && ch != '\'') {
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
                if (!atEOF && ch == ':') {
                    finish = i;
                    readChar();
                    tag = QStringLiteral("op");
                }
                break;
            case '/':
                finish = i;
                readChar();
                if (!atEOF && ch == '/') {
                    do {
                        finish = i;
                        readChar();
                    } while (!atEOF && ch != '\n');
                    tag = QStringLiteral("comment");
                } else if (ch == '*') {
                    bool metAster = false;
                    bool metAsterSlash = false;

                    finish = i;
                    readChar();

                    while (!metAsterSlash) {
                        if (atEOF)
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

        text = QStringView{code}.mid(start, finish - start);
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

    if (start < code.size()) {
        appendProtectedString(&out, QStringView{code}.mid(start));
    }

    return out;
}

QT_END_NAMESPACE
