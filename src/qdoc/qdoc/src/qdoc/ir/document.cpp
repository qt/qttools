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
    Converts the QmlTypeInfo to a QJsonObject for template rendering.

    Always emits \c isSingleton and \c isValueType. The \c importStatement,
    \c inherits, \c inheritedBy, and \c nativeType fields are omitted when
    they don't have values. Templates should use \c{existsIn()} to guard
    access to optional fields.
*/
QJsonObject QmlTypeInfo::toJson() const
{
    QJsonObject json;

    if (!importStatement.isEmpty())
        json["importStatement"_L1] = importStatement;
    json["isSingleton"_L1] = isSingleton;
    json["isValueType"_L1] = isValueType;

    if (inherits) {
        QJsonObject obj;
        obj["name"_L1] = inherits->name;
        obj["href"_L1] = inherits->href;
        obj["moduleName"_L1] = inherits->moduleName;
        json["inherits"_L1] = obj;
    }

    if (!inheritedBy.isEmpty()) {
        QJsonArray arr;
        for (const auto &entry : inheritedBy) {
            QJsonObject obj;
            obj["name"_L1] = entry.name;
            obj["href"_L1] = entry.href;
            arr.append(obj);
        }
        json["inheritedBy"_L1] = arr;
    }

    if (nativeType) {
        QJsonObject obj;
        obj["name"_L1] = nativeType->name;
        obj["href"_L1] = nativeType->href;
        json["nativeType"_L1] = obj;
    }

    return json;
}

static QJsonArray memberEntriesToJson(const QList<CollectionInfo::MemberEntry> &entries)
{
    QJsonArray arr;
    for (const auto &entry : entries) {
        QJsonObject obj;
        obj["name"_L1] = entry.name;
        obj["href"_L1] = entry.href;
        obj["brief"_L1] = entry.brief;
        arr.append(obj);
    }
    return arr;
}

/*!
    Converts the CollectionInfo to a QJsonObject for template rendering.

    Type flags (\c isModule, \c isQmlModule, \c isGroup) and \c noAutoList are
    always emitted so templates can use unconditional checks. CMake/qmake build
    variables are always emitted as empty strings when absent, for template
    safety. Module metadata (\c logicalModuleName, \c logicalModuleVersion,
    \c state) is conditionally emitted when non-empty. Member arrays are always
    emitted (empty arrays when no entries) so Inja can iterate without guards.
*/
QJsonObject CollectionInfo::toJson() const
{
    QJsonObject json;

    json["isModule"_L1] = isModule;
    json["isQmlModule"_L1] = isQmlModule;
    json["isGroup"_L1] = isGroup;
    json["noAutoList"_L1] = noAutoList;

    if (!logicalModuleName.isEmpty())
        json["logicalModuleName"_L1] = logicalModuleName;
    if (!logicalModuleVersion.isEmpty())
        json["logicalModuleVersion"_L1] = logicalModuleVersion;
    if (!state.isEmpty())
        json["state"_L1] = state;

    json["qtVariable"_L1] = qtVariable;
    json["cmakePackage"_L1] = cmakePackage;
    json["cmakeComponent"_L1] = cmakeComponent;
    json["cmakeTargetItem"_L1] = cmakeTargetItem;

    json["namespaces"_L1] = memberEntriesToJson(namespaces);
    json["classes"_L1] = memberEntriesToJson(classes);
    json["members"_L1] = memberEntriesToJson(members);

    return json;
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
    if (!brief.isEmpty())
        json["brief"_L1] = brief;

    Q_ASSERT(!contentJson.contains("blocks"_L1));
    QJsonObject content = contentJson;

    QJsonArray blocks;
    for (const auto &block : body)
        blocks.append(block.toJson());
    content["blocks"_L1] = blocks;

    json["content"_L1] = content;

    // QML type metadata (omitted for non-QML pages)
    json["hasQmlType"_L1] = qmlTypeInfo.has_value();
    if (qmlTypeInfo)
        json["qmlType"_L1] = qmlTypeInfo->toJson();

    // Collection metadata (module, QML module, and group pages).
    json["hasCollection"_L1] = collectionInfo.has_value();
    if (collectionInfo)
        json["collection"_L1] = collectionInfo->toJson();

    // Sections (for aggregate pages with member listings).
    // Always emitted (even when empty) so templates can iterate safely
    // without existsIn() guards on the root data object.
    QJsonArray sectionsArray;
    for (const auto &section : summarySections)
        sectionsArray.append(section.toJson());
    json["sections"_L1] = sectionsArray;

    return json;
}

} // namespace IR

QT_END_NAMESPACE

