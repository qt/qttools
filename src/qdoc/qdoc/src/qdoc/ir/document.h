// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_DOCUMENT_H
#define QDOC_IR_DOCUMENT_H

#include "contentblock.h"
#include "member.h"
#include "signaturespan.h"

#include "qdoc/access.h"
#include "qdoc/genustypes.h"
#include "qdoc/status.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

QT_BEGIN_NAMESPACE

namespace IR {

struct QmlTypeInfo
{
    QString importStatement;
    bool isSingleton { false };
    bool isValueType { false };

    struct InheritsInfo {
        QString name;
        QString href;
        QString moduleName;
    };
    std::optional<InheritsInfo> inherits;

    struct InheritedByEntry {
        QString name;
        QString href;
    };
    QList<InheritedByEntry> inheritedBy;

    struct NativeTypeInfo {
        QString name;
        QString href;
    };
    std::optional<NativeTypeInfo> nativeType;

    [[nodiscard]] QJsonObject toJson() const;
};

struct CollectionInfo
{
    QString logicalModuleName;
    QString logicalModuleVersion;
    QString qtVariable;
    QString cmakePackage;
    QString cmakeComponent;
    QString cmakeTargetItem;
    QString state;

    bool isModule { false };
    bool isQmlModule { false };
    bool isGroup { false };
    bool noAutoList { false };

    struct MemberEntry {
        QString name;
        QString href;
        QString brief;
    };

    QList<MemberEntry> namespaces;
    QList<MemberEntry> classes;
    QList<MemberEntry> members;

    [[nodiscard]] QJsonObject toJson() const;
};

struct CppReferenceInfo
{
    struct BaseClassEntry {
        QString name;
        QString href;
        Access access { Access::Public };
    };

    struct DerivedClassEntry {
        QString name;
        QString href;
    };

    struct QmlNativeTypeLink {
        QString name;
        QString href;
    };

    struct ComparisonEntry {
        QString category;
        QStringList comparableTypes;
        QString description;
    };

    struct ThreadSafetyExceptionEntry {
        QString name;
        QString href;
    };

    struct ThreadSafetyInfo {
        QString level;
        QList<ThreadSafetyExceptionEntry> reentrantExceptions;
        QList<ThreadSafetyExceptionEntry> threadSafeExceptions;
        QList<ThreadSafetyExceptionEntry> nonReentrantExceptions;
    };

    struct GroupEntry {
        QString name;
        QString href;
    };

    QString headerInclude;
    QString cmakeFindPackage;
    QString cmakeTargetLinkLibraries;
    QString qmakeVariable;
    QString statusText;
    QString statusCssClass;
    std::optional<QmlNativeTypeLink> qmlNativeType;
    QList<BaseClassEntry> baseClasses;
    QList<DerivedClassEntry> derivedClasses;
    bool suppressInheritance { false };

    QList<SignatureSpan> templateDeclSpans;

    bool isInnerClass { false };
    bool isNamespace { false };
    bool isHeader { false };

    bool isPartialNamespace { false };
    QString fullNamespaceHref;
    QString fullNamespaceModuleName;

    QString typeWord;
    QStringList ancestorNames;

    QString selfComparisonCategory;
    QList<ComparisonEntry> comparisonEntries;

    std::optional<ThreadSafetyInfo> threadSafety;
    QList<ContentBlock> threadSafetyAdmonition;

    QList<GroupEntry> groups;

    bool hasObsoleteMembers { false };
    QString obsoleteMembersUrl;

    [[nodiscard]] QJsonObject toJson() const;
};

struct NavigationInfo
{
    enum class CrumbState {
        Link,
        Current,
        Unresolved,
    };

    struct BreadcrumbEntry {
        QString title;
        QString href;
        CrumbState state { CrumbState::Link };
    };

    struct LinkEntry {
        QString title;
        QString href;
    };

    struct TocEntry {
        QString title;
        QString anchorId;
        int level { 2 };
    };

    QList<BreadcrumbEntry> breadcrumbs;
    std::optional<LinkEntry> previousLink;
    std::optional<LinkEntry> nextLink;
    std::optional<LinkEntry> startLink;
    QList<TocEntry> tocEntries;
    int tocDepth { -1 };

    [[nodiscard]] QJsonObject toJson() const;
};

struct Document
{
    // Classification
    NodeType nodeType { NodeType::NoType };
    Genus genus { Genus::DontCare };
    Status status { Status::Active };
    Access access { Access::Public };

    // Identity
    QString title;              // Page title
    QString fullTitle;          // Full qualified title
    QString url;                // Page URL: canonical absolute when the
                                // 'url' qdocconf is set; output filename
                                // (relative) otherwise.
    QString since;              // Version introduced (e.g., "6.8")
    QString deprecatedSince;    // Version deprecated (e.g., "6.5")
    QString brief;              // Brief description

    // Content
    QList<ContentBlock> body;
    QJsonObject contentJson;

    // Members (for aggregate pages)
    QList<SectionIR> summarySections;
    QList<SectionIR> detailSections;

    // QML type metadata (populated only for QML type pages)
    std::optional<QmlTypeInfo> qmlTypeInfo;

    // Collection metadata (populated for module, QML module, and group pages)
    std::optional<CollectionInfo> collectionInfo;

    // C++ reference metadata (populated for class, namespace, and header pages)
    std::optional<CppReferenceInfo> cppReferenceInfo;

    // Navigation metadata (populated for all pages with navigation config)
    std::optional<NavigationInfo> navigationInfo;

    // Members sub-page URL (set when a members listing page is generated)
    QString membersPageUrl;

    QJsonObject toJson() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_DOCUMENT_H

