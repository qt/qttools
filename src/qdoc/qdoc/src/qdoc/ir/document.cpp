// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "document.h"

#include <QJsonArray>
#include <optional>

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

// Classification fields use a two-part structure:
// - "id": Stable kebab-case identifier for template conditionals (e.g., "qml-type")
// - "label": Human-readable display name (e.g., "QML type")
//
// This separation allows templates to use stable ids for logic while displaying
// user-friendly labels. Changing labels won't break template conditionals.

// Helper to create a classification JSON object with id and label
static QJsonObject classificationObject(const QString &id, const QString &label)
{
    QJsonObject obj;
    obj["id"_L1] = id;
    obj["label"_L1] = label;
    return obj;
}

// Returns {id, label} pair for NodeType. Returns nullopt for NoType (unclassified).
static std::optional<QJsonObject> nodeTypeToJson(NodeType t)
{
    switch (t) {
    case NodeType::NoType:        return std::nullopt;
    case NodeType::Namespace:     return classificationObject("namespace"_L1, "Namespace"_L1);
    case NodeType::Class:         return classificationObject("class"_L1, "Class"_L1);
    case NodeType::Struct:        return classificationObject("struct"_L1, "Struct"_L1);
    case NodeType::Union:         return classificationObject("union"_L1, "Union"_L1);
    case NodeType::HeaderFile:    return classificationObject("header-file"_L1, "Header file"_L1);
    case NodeType::Page:          return classificationObject("page"_L1, "Page"_L1);
    case NodeType::Enum:          return classificationObject("enum"_L1, "Enum"_L1);
    case NodeType::Example:       return classificationObject("example"_L1, "Example"_L1);
    case NodeType::ExternalPage:  return classificationObject("external-page"_L1, "External page"_L1);
    case NodeType::TypeAlias:     return classificationObject("type-alias"_L1, "Type alias"_L1);
    case NodeType::Typedef:       return classificationObject("typedef"_L1, "Typedef"_L1);
    case NodeType::Function:      return classificationObject("function"_L1, "Function"_L1);
    case NodeType::Property:      return classificationObject("property"_L1, "Property"_L1);
    case NodeType::Proxy:         return classificationObject("proxy"_L1, "Proxy"_L1);
    case NodeType::Variable:      return classificationObject("variable"_L1, "Variable"_L1);
    case NodeType::Group:         return classificationObject("group"_L1, "Group"_L1);
    case NodeType::Module:        return classificationObject("module"_L1, "Module"_L1);
    case NodeType::QmlType:       return classificationObject("qml-type"_L1, "QML type"_L1);
    case NodeType::QmlValueType:  return classificationObject("qml-value-type"_L1, "QML value type"_L1);
    case NodeType::QmlModule:     return classificationObject("qml-module"_L1, "QML module"_L1);
    case NodeType::QmlProperty:   return classificationObject("qml-property"_L1, "QML property"_L1);
    case NodeType::QmlEnum:       return classificationObject("qml-enum"_L1, "QML enum"_L1);
    case NodeType::SharedComment: return classificationObject("shared-comment"_L1, "Shared comment"_L1);
    case NodeType::Collection:    return classificationObject("collection"_L1, "Collection"_L1);
    }
    Q_UNREACHABLE();
}

// Returns {id, label} pair for Genus. Returns nullopt for DontCare (unclassified).
static std::optional<QJsonObject> genusToJson(Genus g)
{
    switch (g) {
    case Genus::DontCare: return std::nullopt;
    case Genus::CPP:      return classificationObject("cpp"_L1, "C++"_L1);
    case Genus::QML:      return classificationObject("qml"_L1, "QML"_L1);
    case Genus::DOC:      return classificationObject("doc"_L1, "Documentation"_L1);
    case Genus::API:      return classificationObject("api"_L1, "API"_L1);
    }
    Q_UNREACHABLE();
}

// Returns {id, label} pair for Status.
static QJsonObject statusToJson(Status s)
{
    switch (s) {
    case Status::Deprecated:   return classificationObject("deprecated"_L1, "Deprecated"_L1);
    case Status::Preliminary:  return classificationObject("preliminary"_L1, "Preliminary"_L1);
    case Status::Active:       return classificationObject("active"_L1, "Active"_L1);
    case Status::Internal:     return classificationObject("internal"_L1, "Internal"_L1);
    case Status::DontDocument: return classificationObject("ignored"_L1, "Ignored"_L1);
    }
    Q_UNREACHABLE();
}

// Returns {id, label} pair for Access.
static QJsonObject accessToJson(Access a)
{
    switch (a) {
    case Access::Public:    return classificationObject("public"_L1, "Public"_L1);
    case Access::Protected: return classificationObject("protected"_L1, "Protected"_L1);
    case Access::Private:   return classificationObject("private"_L1, "Private"_L1);
    }
    Q_UNREACHABLE();
}

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

