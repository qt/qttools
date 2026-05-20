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

    Type flags (\c isModule, \c isQmlModule, \c isGroup, \c isConcept) and \c noAutoList are
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
    json["isConcept"_L1] = isConcept;
    json["noAutoList"_L1] = noAutoList;

    if (!logicalModuleName.isEmpty())
        json["logicalModuleName"_L1] = logicalModuleName;
    if (!logicalModuleVersion.isEmpty())
        json["logicalModuleVersion"_L1] = logicalModuleVersion;
    if (!state.isEmpty())
        json["state"_L1] = state;

    if (!qtVariable.isEmpty())
        json["qtVariable"_L1] = qtVariable;
    if (!cmakePackage.isEmpty())
        json["cmakePackage"_L1] = cmakePackage;
    if (!cmakeComponent.isEmpty())
        json["cmakeComponent"_L1] = cmakeComponent;
    if (!cmakeTargetItem.isEmpty())
        json["cmakeTargetItem"_L1] = cmakeTargetItem;

    json["namespaces"_L1] = memberEntriesToJson(namespaces);
    json["classes"_L1] = memberEntriesToJson(classes);
    json["members"_L1] = memberEntriesToJson(members);

    return json;
}

/*!
    Converts CppReferenceInfo to a QJsonObject for template rendering.

    Boolean flags and list fields are always emitted so templates can
    iterate without guards. String fields are omitted when empty.
    The access specifier on base class entries uses the {id, label}
    convention from classificationjson.h.
*/
QJsonObject CppReferenceInfo::toJson() const
{
    QJsonObject json;

    if (!headerInclude.isEmpty())
        json["headerInclude"_L1] = headerInclude;
    if (!cmakeFindPackage.isEmpty())
        json["cmakeFindPackage"_L1] = cmakeFindPackage;
    if (!cmakeTargetLinkLibraries.isEmpty())
        json["cmakeTargetLinkLibraries"_L1] = cmakeTargetLinkLibraries;
    if (!qmakeVariable.isEmpty())
        json["qmakeVariable"_L1] = qmakeVariable;
    if (!statusText.isEmpty())
        json["statusText"_L1] = statusText;
    if (!statusCssClass.isEmpty())
        json["statusCssClass"_L1] = statusCssClass;

    if (qmlNativeType) {
        QJsonObject obj;
        obj["name"_L1] = qmlNativeType->name;
        obj["href"_L1] = qmlNativeType->href;
        json["qmlNativeType"_L1] = obj;
    }

    QJsonArray baseClassesArr;
    for (const auto &entry : baseClasses) {
        QJsonObject obj;
        obj["name"_L1] = entry.name;
        obj["href"_L1] = entry.href;
        obj["access"_L1] = accessToJson(entry.access);
        baseClassesArr.append(obj);
    }
    json["baseClasses"_L1] = baseClassesArr;

    QJsonArray derivedClassesArr;
    for (const auto &entry : derivedClasses) {
        QJsonObject obj;
        obj["name"_L1] = entry.name;
        obj["href"_L1] = entry.href;
        derivedClassesArr.append(obj);
    }
    json["derivedClasses"_L1] = derivedClassesArr;

    json["suppressInheritance"_L1] = suppressInheritance;

    QJsonArray templateDeclArr;
    for (const auto &span : templateDeclSpans)
        templateDeclArr.append(span.toJson());
    json["templateDeclSpans"_L1] = templateDeclArr;

    json["isInnerClass"_L1] = isInnerClass;
    json["isNamespace"_L1] = isNamespace;
    json["isHeader"_L1] = isHeader;

    json["isPartialNamespace"_L1] = isPartialNamespace;
    if (!fullNamespaceHref.isEmpty())
        json["fullNamespaceHref"_L1] = fullNamespaceHref;
    if (!fullNamespaceModuleName.isEmpty())
        json["fullNamespaceModuleName"_L1] = fullNamespaceModuleName;

    json["typeWord"_L1] = typeWord;

    QJsonArray ancestorNamesArr;
    for (const auto &name : ancestorNames)
        ancestorNamesArr.append(name);
    json["ancestorNames"_L1] = ancestorNamesArr;

    if (!selfComparisonCategory.isEmpty())
        json["selfComparisonCategory"_L1] = selfComparisonCategory;

    QJsonArray comparisonArr;
    for (const auto &entry : comparisonEntries) {
        QJsonObject obj;
        obj["category"_L1] = entry.category;
        QJsonArray typesArr;
        for (const auto &t : entry.comparableTypes)
            typesArr.append(t);
        obj["comparableTypes"_L1] = typesArr;
        obj["description"_L1] = entry.description;
        comparisonArr.append(obj);
    }
    json["comparisonEntries"_L1] = comparisonArr;

    if (threadSafety) {
        QJsonObject tsObj;
        tsObj["level"_L1] = threadSafety->level;

        auto exceptionListToJson = [](const QList<ThreadSafetyExceptionEntry> &entries) {
            QJsonArray arr;
            for (const auto &entry : entries) {
                QJsonObject obj;
                obj["name"_L1] = entry.name;
                obj["href"_L1] = entry.href;
                arr.append(obj);
            }
            return arr;
        };

        tsObj["reentrantExceptions"_L1] = exceptionListToJson(threadSafety->reentrantExceptions);
        tsObj["threadSafeExceptions"_L1] = exceptionListToJson(threadSafety->threadSafeExceptions);
        tsObj["nonReentrantExceptions"_L1] = exceptionListToJson(threadSafety->nonReentrantExceptions);
        json["threadSafety"_L1] = tsObj;
    }

    if (!threadSafetyAdmonition.isEmpty()) {
        QJsonArray admonitionArr;
        for (const auto &block : threadSafetyAdmonition)
            admonitionArr.append(block.toJson());
        json["threadSafetyAdmonition"_L1] = admonitionArr;
    }

    QJsonArray groupsArr;
    for (const auto &entry : groups) {
        QJsonObject obj;
        obj["name"_L1] = entry.name;
        obj["href"_L1] = entry.href;
        groupsArr.append(obj);
    }
    json["groups"_L1] = groupsArr;

    json["hasObsoleteMembers"_L1] = hasObsoleteMembers;
    if (!obsoleteMembersUrl.isEmpty())
        json["obsoleteMembersUrl"_L1] = obsoleteMembersUrl;

    return json;
}

static QString crumbStateString(NavigationInfo::CrumbState state)
{
    switch (state) {
    case NavigationInfo::CrumbState::Link:
        return u"link"_s;
    case NavigationInfo::CrumbState::Current:
        return u"current"_s;
    case NavigationInfo::CrumbState::Unresolved:
        return u"unresolved"_s;
    }
    Q_UNREACHABLE_RETURN(u"link"_s);
}

/*!
    Converts NavigationInfo to a QJsonObject for template rendering.

    The \c breadcrumbs and \c tocEntries arrays are always emitted (empty
    when none exist) so templates can iterate without guards. Each breadcrumb
    carries a \c state discriminator (\c link, \c current, or \c unresolved)
    so templates can distinguish a resolvable link, the current page, and
    an unresolvable ancestor without conflating them through an empty href.
    Sequential link objects (\c prevLink, \c nextLink, \c startLink) are
    conditionally emitted only when set. The \c tocDepth integer is always
    emitted (\c{-1} means unlimited depth).
*/
QJsonObject NavigationInfo::toJson() const
{
    QJsonObject json;

    QJsonArray breadcrumbArr;
    for (const auto &entry : breadcrumbs) {
        QJsonObject obj;
        obj["title"_L1] = entry.title;
        obj["href"_L1] = entry.href;
        obj["state"_L1] = crumbStateString(entry.state);
        breadcrumbArr.append(obj);
    }
    json["breadcrumbs"_L1] = breadcrumbArr;

    if (previousLink) {
        QJsonObject obj;
        obj["title"_L1] = previousLink->title;
        obj["href"_L1] = previousLink->href;
        json["prevLink"_L1] = obj;
    }

    if (nextLink) {
        QJsonObject obj;
        obj["title"_L1] = nextLink->title;
        obj["href"_L1] = nextLink->href;
        json["nextLink"_L1] = obj;
    }

    if (startLink) {
        QJsonObject obj;
        obj["title"_L1] = startLink->title;
        obj["href"_L1] = startLink->href;
        json["startLink"_L1] = obj;
    }

    QJsonArray tocArr;
    for (const auto &entry : tocEntries) {
        QJsonObject obj;
        obj["title"_L1] = entry.title;
        obj["anchorId"_L1] = entry.anchorId;
        obj["level"_L1] = entry.level;
        tocArr.append(obj);
    }
    json["tocEntries"_L1] = tocArr;

    json["tocDepth"_L1] = tocDepth;

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

    // C++ reference metadata (class, namespace, and header pages).
    json["hasCppRef"_L1] = cppReferenceInfo.has_value();
    if (cppReferenceInfo)
        json["cppRef"_L1] = cppReferenceInfo->toJson();

    // Navigation metadata (breadcrumbs, sequential links, TOC depth).
    json["hasNavigation"_L1] = navigationInfo.has_value();
    if (navigationInfo)
        json["navigation"_L1] = navigationInfo->toJson();

    // Members sub-page URL (always emitted for Inja root-level variable safety;
    // empty string when no members sub-page was generated)
    json["membersPageUrl"_L1] = membersPageUrl;

    // Sections (for aggregate pages with member listings).
    // Always emitted (even when empty) so templates can iterate safely
    // without existsIn() guards on the root data object.
    QJsonArray sectionsArray;
    for (const auto &section : summarySections)
        sectionsArray.append(section.toJson());
    json["sections"_L1] = sectionsArray;

    QJsonArray detailSectionsArray;
    for (const auto &section : detailSections)
        detailSectionsArray.append(section.toJson());
    json["detailSections"_L1] = detailSectionsArray;

    return json;
}

} // namespace IR

QT_END_NAMESPACE

