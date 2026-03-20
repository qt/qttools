// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "document.h"

#include "classificationjson.h"

#include <QJsonArray>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \struct IR::Document
    \brief Intermediate representation for a documentation topic.

    Document contains all information needed to render a single documentation
    page using templates. All links are pre-resolved, sections are pre-organized,
    and file paths are pre-computed. The template engine receives only this IR
    and performs no lookups or resolution itself.

    The struct includes classification metadata (nodeType, genus, status, access)
    that allows templates to conditionally render content based on the type and
    visibility of the documented entity.
*/

/*!
    Converts the Document to a QJsonObject for template rendering.

    The JSON structure follows a convention where field names use camelCase
    and match template variable names. Classification fields (nodeType, genus,
    status, access) use a two-part structure with "id" (stable kebab-case
    identifier for conditionals) and "label" (human-readable display name).

    The \c contentJson field is nested under a 'content' key to provide better
    structure and namespace separation in templates.

    Returns a QJsonObject containing all IR data in a format suitable for
    passing to the Inja template engine via InjaBridge.
*/
QJsonObject Document::toJson() const
{
    QJsonObject json;

    // Classification (as {id, label} objects for template convenience)
    // nodeType and genus are omitted when unclassified (NoType/DontCare),
    // allowing templates to use `if defined` checks.
    if (const auto t = nodeTypeToJson(nodeType))
        json["nodeType"_L1] = *t;
    if (const auto g = genusToJson(genus))
        json["genus"_L1] = *g;
    json["status"_L1] = statusToJson(status);
    json["access"_L1] = accessToJson(access);

    // Identity
    json["title"_L1] = title;
    json["fullTitle"_L1] = fullTitle;
    json["url"_L1] = url;
    if (!since.isEmpty())
        json["since"_L1] = since;
    if (!deprecatedSince.isEmpty())
        json["deprecatedSince"_L1] = deprecatedSince;
    json["brief"_L1] = brief;

    Q_ASSERT(!contentJson.contains("blocks"_L1));
    QJsonObject content = contentJson;

    QJsonArray blocks;
    for (const auto &block : body)
        blocks.append(block.toJson());
    content["blocks"_L1] = blocks;

    json["content"_L1] = content;

    return json;
}

} // namespace IR

QT_END_NAMESPACE

