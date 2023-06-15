// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "codemarker.h"

#include "classnode.h"
#include "config.h"
#include "functionnode.h"
#include "node.h"
#include "propertynode.h"

#include <QtCore/qobjectdefs.h>

QT_BEGIN_NAMESPACE

QString CodeMarker::s_defaultLang;
QList<CodeMarker *> CodeMarker::s_markers;

/*!
  When a code marker constructs itself, it puts itself into
  the static list of code markers. All the code markers in
  the static list get initialized in initialize(), which is
  not called until after the qdoc configuration file has
  been read.
 */
CodeMarker::CodeMarker()
{
    s_markers.prepend(this);
}

/*!
  When a code marker destroys itself, it removes itself from
  the static list of code markers.
 */
CodeMarker::~CodeMarker()
{
    s_markers.removeAll(this);
}

/*!
  A code market performs no initialization by default. Marker-specific
  initialization is performed in subclasses.
 */
void CodeMarker::initializeMarker() {}

/*!
  Terminating a code marker is trivial.
 */
void CodeMarker::terminateMarker()
{
    // nothing.
}

/*!
  All the code markers in the static list are initialized
  here, after the qdoc configuration file has been loaded.
 */
void CodeMarker::initialize()
{
    s_defaultLang = Config::instance().get(CONFIG_LANGUAGE).asString();
    for (const auto &marker : std::as_const(s_markers))
        marker->initializeMarker();
}

/*!
  All the code markers in the static list are terminated here.
 */
void CodeMarker::terminate()
{
    for (const auto &marker : std::as_const(s_markers))
        marker->terminateMarker();
}

CodeMarker *CodeMarker::markerForCode(const QString &code)
{
    CodeMarker *defaultMarker = markerForLanguage(s_defaultLang);
    if (defaultMarker != nullptr && defaultMarker->recognizeCode(code))
        return defaultMarker;

    for (const auto &marker : std::as_const(s_markers)) {
        if (marker->recognizeCode(code))
            return marker;
    }

    return defaultMarker;
}

CodeMarker *CodeMarker::markerForFileName(const QString &fileName)
{
    CodeMarker *defaultMarker = markerForLanguage(s_defaultLang);
    qsizetype dot = -1;
    while ((dot = fileName.lastIndexOf(QLatin1Char('.'), dot)) != -1) {
        QString ext = fileName.mid(dot + 1);
        if (defaultMarker != nullptr && defaultMarker->recognizeExtension(ext))
            return defaultMarker;
        for (const auto &marker : std::as_const(s_markers)) {
            if (marker->recognizeExtension(ext))
                return marker;
        }
        --dot;
    }
    return defaultMarker;
}

CodeMarker *CodeMarker::markerForLanguage(const QString &lang)
{
    for (const auto &marker : std::as_const(s_markers)) {
        if (marker->recognizeLanguage(lang))
            return marker;
    }
    return nullptr;
}

const Node *CodeMarker::nodeForString(const QString &string)
{
#if QT_POINTER_SIZE == 4
    const quintptr n = string.toUInt();
#else
    const quintptr n = string.toULongLong();
#endif
    return reinterpret_cast<const Node *>(n);
}

QString CodeMarker::stringForNode(const Node *node)
{
    return QString::number(reinterpret_cast<quintptr>(node));
}

/*!
    Returns the 'extra' synopsis string for \a node with status information,
    using a specified section \a style.
*/
QString CodeMarker::extraSynopsis(const Node *node, Section::Style style)
{
    QStringList extra;
    if (style == Section::Details) {
        switch (node->nodeType()) {
        case Node::Function: {
            const auto *func = static_cast<const FunctionNode *>(node);
            if (func->isStatic()) {
                extra << "static";
            } else if (!func->isNonvirtual()) {
                if (func->isFinal())
                    extra << "final";
                if (func->isOverride())
                    extra << "override";
                if (func->isPureVirtual())
                    extra << "pure";
                extra << "virtual";
            }

            if (func->isExplicit()) extra << "explicit";
            if (func->isConstexpr()) extra << "constexpr";
            if (auto noexcept_info = func->getNoexcept()) {
                extra << (QString("noexcept") + (!(*noexcept_info).isEmpty() ? "(...)" : ""));
            }

            if (func->access() == Access::Protected)
                extra << "protected";
            else if (func->access() == Access::Private)
                extra << "private";

            if (func->isSignal()) {
                if (func->parameters().isPrivateSignal())
                    extra << "private";
                extra << "signal";
            } else if (func->isSlot())
                extra << "slot";
            else if (func->isDefault())
                extra << "default";
            else if (func->isInvokable())
                extra << "invokable";
        }
        break;
        case Node::TypeAlias:
            extra << "alias";
            break;
        case Node::Property: {
            auto propertyNode = static_cast<const PropertyNode *>(node);
            if (propertyNode->propertyType() == PropertyNode::PropertyType::BindableProperty)
                extra << "bindable";
            if (!propertyNode->isWritable())
                extra << "read-only";
        }
        break;
        default:
            break;
        }
    } else if (style == Section::Summary) {
        if (node->isPreliminary())
            extra << "preliminary";
        else if (node->isDeprecated()) {
            extra << "deprecated";
            if (const QString &since = node->deprecatedSince(); !since.isEmpty())
                extra << QStringLiteral("(%1)").arg(since);
        }
    }

    if (style == Section::Details && !node->since().isEmpty()) {
        if (!extra.isEmpty())
            extra.last() += QLatin1Char(',');
        extra << "since" << node->since();
    }

    QString extraStr = extra.join(QLatin1Char(' '));
    if (!extraStr.isEmpty()) {
        extraStr.prepend(style == Section::Details ? '[' : '(');
        extraStr.append(style == Section::Details ? ']' : ')');
        extraStr.append(' ');
    }

    return extraStr;
}

static const QString samp = QLatin1String("&amp;");
static const QString slt = QLatin1String("&lt;");
static const QString sgt = QLatin1String("&gt;");
static const QString squot = QLatin1String("&quot;");

QString CodeMarker::protect(const QString &str)
{
    qsizetype n = str.size();
    QString marked;
    marked.reserve(n * 2 + 30);
    const QChar *data = str.constData();
    for (int i = 0; i != n; ++i) {
        switch (data[i].unicode()) {
        case '&':
            marked += samp;
            break;
        case '<':
            marked += slt;
            break;
        case '>':
            marked += sgt;
            break;
        case '"':
            marked += squot;
            break;
        default:
            marked += data[i];
        }
    }
    return marked;
}

void CodeMarker::appendProtectedString(QString *output, QStringView str)
{
    qsizetype n = str.size();
    output->reserve(output->size() + n * 2 + 30);
    const QChar *data = str.constData();
    for (int i = 0; i != n; ++i) {
        switch (data[i].unicode()) {
        case '&':
            *output += samp;
            break;
        case '<':
            *output += slt;
            break;
        case '>':
            *output += sgt;
            break;
        case '"':
            *output += squot;
            break;
        default:
            *output += data[i];
        }
    }
}

QString CodeMarker::typified(const QString &string, bool trailingSpace)
{
    QString result;
    QString pendingWord;

    for (int i = 0; i <= string.size(); ++i) {
        QChar ch;
        if (i != string.size())
            ch = string.at(i);

        QChar lower = ch.toLower();
        if ((lower >= QLatin1Char('a') && lower <= QLatin1Char('z')) || ch.digitValue() >= 0
            || ch == QLatin1Char('_') || ch == QLatin1Char(':')) {
            pendingWord += ch;
        } else {
            if (!pendingWord.isEmpty()) {
                bool isProbablyType = (pendingWord != QLatin1String("const"));
                if (isProbablyType)
                    result += QLatin1String("<@type>");
                result += pendingWord;
                if (isProbablyType)
                    result += QLatin1String("</@type>");
            }
            pendingWord.clear();

            switch (ch.unicode()) {
            case '\0':
                break;
            case '&':
                result += QLatin1String("&amp;");
                break;
            case '<':
                result += QLatin1String("&lt;");
                break;
            case '>':
                result += QLatin1String("&gt;");
                break;
            default:
                result += ch;
            }
        }
    }
    if (trailingSpace && string.size()) {
        if (!string.endsWith(QLatin1Char('*')) && !string.endsWith(QLatin1Char('&')))
            result += QLatin1Char(' ');
    }
    return result;
}

QString CodeMarker::taggedNode(const Node *node)
{
    QString tag;
    const QString &name = node->name();

    switch (node->nodeType()) {
    case Node::Namespace:
        tag = QLatin1String("@namespace");
        break;
    case Node::Class:
    case Node::Struct:
    case Node::Union:
        tag = QLatin1String("@class");
        break;
    case Node::Enum:
        tag = QLatin1String("@enum");
        break;
    case Node::TypeAlias:
    case Node::Typedef:
        tag = QLatin1String("@typedef");
        break;
    case Node::Function:
        tag = QLatin1String("@function");
        break;
    case Node::Property:
        tag = QLatin1String("@property");
        break;
    case Node::QmlType:
        tag = QLatin1String("@property");
        break;
    case Node::Page:
        tag = QLatin1String("@property");
        break;
    default:
        tag = QLatin1String("@unknown");
        break;
    }
    return (QLatin1Char('<') + tag + QLatin1Char('>') + protect(name) + QLatin1String("</") + tag
            + QLatin1Char('>'));
}

QString CodeMarker::taggedQmlNode(const Node *node)
{
    QString tag;
    if (node->isFunction()) {
        const auto *fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::QmlSignal:
            tag = QLatin1String("@signal");
            break;
        case FunctionNode::QmlSignalHandler:
            tag = QLatin1String("@signalhandler");
            break;
        case FunctionNode::QmlMethod:
            tag = QLatin1String("@method");
            break;
        default:
            tag = QLatin1String("@unknown");
            break;
        }
    } else if (node->isQmlProperty()) {
        tag = QLatin1String("@property");
    } else {
        tag = QLatin1String("@unknown");
    }
    return QLatin1Char('<') + tag + QLatin1Char('>') + protect(node->name()) + QLatin1String("</")
            + tag + QLatin1Char('>');
}

QString CodeMarker::linkTag(const Node *node, const QString &body)
{
    return QLatin1String("<@link node=\"") + stringForNode(node) + QLatin1String("\">") + body
            + QLatin1String("</@link>");
}

QT_END_NAMESPACE
