// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "codemarker.h"

#include "classnode.h"
#include "config.h"
#include "functionnode.h"
#include "enumnode.h"
#include "genustypes.h"
#include "propertynode.h"
#include "qmlpropertynode.h"
#include "utilities.h"

#include <QtCore/qobjectdefs.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

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

/*!
    Returns a string representing the \a node status, set using \preliminary, \since,
    and \deprecated commands.

    If a string is returned, it is one of:
    \list
        \li \c {"preliminary"}
        \li \c {"since <version_since>, deprecated in <version_deprecated>"}
        \li \c {"since <version_since>, until <version_deprecated>"}
        \li \c {"since <version_since>"}
        \li \c {"since <version_since>, deprecated"}
        \li \c {"deprecated in <version_deprecated>"}
        \li \c {"until <version_deprecated>"}
        \li \c {"deprecated"}
    \endlist

    If \a node has no related status information, returns std::nullopt.
*/
static std::optional<QString> nodeStatusAsString(const Node *node)
{
    if (node->isPreliminary())
        return std::optional(u"preliminary"_s);

    QStringList result;
    if (const auto &since = node->since(); !since.isEmpty())
        result << "since %1"_L1.arg(since);
    if (const auto &deprecated = node->deprecatedSince(); !deprecated.isEmpty()) {
        if (node->isDeprecated())
            result << "deprecated in %1"_L1.arg(deprecated);
        else
            result << "until %1"_L1.arg(deprecated);
    } else if (node->isDeprecated()) {
        result << u"deprecated"_s;
    }

    return result.isEmpty() ? std::nullopt : std::optional(result.join(u", "_s));
}

/*!
    Returns the 'extra' synopsis string for \a node with status information,
    using a specified section \a style.
*/
QString CodeMarker::extraSynopsis(const Node *node, Section::Style style)
{
    if (style != Section::Summary && style != Section::Details)
        return {};

    QStringList extra;
    if (style == Section::Details) {
        switch (node->nodeType()) {
        case NodeType::Enum:
            if (static_cast<const EnumNode *>(node)->isAnonymous())
                extra << "anonymous";
            break;
        case NodeType::Function: {
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
        case NodeType::TypeAlias:
            extra << "alias";
            break;
        case NodeType::Property: {
            auto propertyNode = static_cast<const PropertyNode *>(node);
            if (propertyNode->propertyType() == PropertyNode::PropertyType::BindableProperty)
                extra << "bindable";
            if (!propertyNode->isWritable())
                extra << "read-only";
        }
        break;
        case NodeType::QmlProperty: {
            auto qmlProperty = static_cast<const QmlPropertyNode *>(node);
            if (qmlProperty->isDefault())
                extra << u"default"_s;
            // Call non-const overloads to ensure attributes are fetched from
            // associated C++ properties
            else if (const_cast<QmlPropertyNode *>(qmlProperty)->isReadOnly())
                extra << u"read-only"_s;
            else if (const_cast<QmlPropertyNode *>(qmlProperty)->isRequired())
                extra << u"required"_s;
            else if (!qmlProperty->defaultValue().isEmpty()) {
                extra << u"default: "_s + qmlProperty->defaultValue();
        }
        break;
        }
        default:
            break;
        }
    }

    // Add status for both Summary and Details
    if (auto status = nodeStatusAsString(node)) {
        if (!extra.isEmpty())
            extra.last() += ','_L1;
        extra << *status;
    }

    QString extraStr = extra.join(QLatin1Char(' '));
    if (!extraStr.isEmpty()) {
        extraStr.prepend(style == Section::Details ? '[' : '(');
        extraStr.append(style == Section::Details ? ']' : ')');
    }

    return extraStr;
}

QString CodeMarker::protect(const QString &str)
{
    return Utilities::protect(str);
}

void CodeMarker::appendProtectedString(QString *output, QStringView str)
{
    qsizetype n = str.size();
    output->reserve(output->size() + n * 2 + 30);
    const QChar *data = str.constData();
    for (int i = 0; i != n; ++i) {
        switch (data[i].unicode()) {
        case '&':
            *output += Utilities::samp;
            break;
        case '<':
            *output += Utilities::slt;
            break;
        case '>':
            *output += Utilities::sgt;
            break;
        case '"':
            *output += Utilities::squot;
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
    case NodeType::Namespace:
        tag = QLatin1String("@namespace");
        break;
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        tag = QLatin1String("@class");
        break;
    case NodeType::Enum:
        tag = QLatin1String("@enum");
        break;
    case NodeType::TypeAlias:
    case NodeType::Typedef:
        tag = QLatin1String("@typedef");
        break;
    case NodeType::Function:
        tag = QLatin1String("@function");
        break;
    case NodeType::Property:
        tag = QLatin1String("@property");
        break;
    case NodeType::QmlType:
        tag = QLatin1String("@property");
        break;
    case NodeType::Page:
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
    return QLatin1String("<@link node=\"") + Utilities::stringForNode(node) + QLatin1String("\">") + body
            + QLatin1String("</@link>");
}

QT_END_NAMESPACE
