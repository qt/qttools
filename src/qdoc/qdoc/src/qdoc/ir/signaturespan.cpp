// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "signaturespan.h"

#include <QJsonArray>
#include <QStringList>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \enum IR::SpanRole
    \internal
    \brief Discriminator for semantic roles within a signature span sequence.

    SpanRole classifies each span in a signature representation by its
    semantic purpose. Renderers use the role to apply appropriate formatting
    (e.g., linking type names, styling qualifiers as badges).

    \value Text Plain text such as punctuation, whitespace, or keywords like "const".
    \value Type A type name, potentially linkable to its documentation page.
    \value Name The member or function name, potentially linkable.
    \value Parameter A parameter name within a function signature.
    \value Operator A delimiter operator such as "::" or ".".
    \value Extra An extra qualifier badge such as "[static]" or "[virtual]".
    \value TemplateDecl A template declaration wrapper containing child type spans.
    \value Link An explicit link to a documented node.
    \value ExternalRef An external reference such as a cppreference.com link.
*/

/*!
    \struct IR::SignatureSpan
    \internal
    \brief Represents a single span within a structured signature.

    SignatureSpan is a format-agnostic representation of one semantic
    element within a function or member signature. Spans carry a semantic
    role, display text, an optional link target, and optional children
    for nested structures such as template declarations.

    Unlike CodeMarker's tagged strings, SignatureSpan captures semantic
    knowledge directly in the IR. Any generator consuming the IR can
    render signatures correctly without calling back into QDoc internals.

    This is a pure value type with no dependencies on QDoc's core
    infrastructure. It belongs in QDocLib.

    \sa SpanRole
*/

static QString spanRoleId(SpanRole role)
{
    switch (role) {
    case SpanRole::Text:         return u"text"_s;
    case SpanRole::Type:         return u"type"_s;
    case SpanRole::Name:         return u"name"_s;
    case SpanRole::Parameter:    return u"parameter"_s;
    case SpanRole::Operator:     return u"operator"_s;
    case SpanRole::Extra:        return u"extra"_s;
    case SpanRole::TemplateDecl: return u"template-decl"_s;
    case SpanRole::Link:         return u"link"_s;
    case SpanRole::ExternalRef:  return u"external-ref"_s;
    }
    Q_UNREACHABLE();
}

/*!
    Converts the SignatureSpan to a QJsonObject for template rendering.

    The JSON includes a \c role key with the kebab-case role identifier
    and a \c text key with the span's display text. The \c href key is
    omitted when empty. The \c children array is omitted when empty.
*/
QJsonObject SignatureSpan::toJson() const
{
    QJsonObject json;
    json["role"_L1] = spanRoleId(role);
    json["text"_L1] = text;

    if (!href.isEmpty())
        json["href"_L1] = href;

    if (!children.isEmpty()) {
        QJsonArray childArr;
        for (const auto &child : children)
            childArr.append(child.toJson());
        json["children"_L1] = childArr;
    }

    return json;
}

/*!
    Returns the concatenated plain text of this span and all its
    children, recursively.

    The span's own text is emitted first, followed by the concatenated
    plain text of all children. No separators are inserted — spans
    carry their own whitespace as Text spans.
*/
QString SignatureSpan::plainText() const
{
    if (children.isEmpty())
        return text;

    QStringList parts;
    parts.reserve(children.size() + 1);

    if (!text.isEmpty())
        parts.append(text);

    for (const auto &child : children)
        parts.append(child.plainText());

    return parts.join(u""_s);
}

} // namespace IR

QT_END_NAMESPACE
