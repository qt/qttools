// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_PAGEMETADATA_H
#define QDOC_IR_PAGEMETADATA_H

#include "contentblock.h"
#include "member.h"
#include "signaturespan.h"

#include "qdoc/access.h"
#include "qdoc/genustypes.h"
#include "qdoc/status.h"

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <optional>

QT_BEGIN_NAMESPACE

namespace IR {

struct QmlTypeData
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
};

struct CollectionData
{
    struct MemberEntry {
        QString name;
        QString href;
        QString brief;
    };

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
    bool isConcept { false };
    bool noAutoList { false };

    QList<MemberEntry> namespaces;
    QList<MemberEntry> classes;
    QList<MemberEntry> members;
};

struct CppReferenceData
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
    QList<QString> referencedConcepts;

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

    QList<GroupEntry> groups;

    bool hasObsoleteMembers { false };
};

struct NavigationData
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
};

struct PageMetadata
{
    NodeType nodeType { NodeType::NoType };
    Genus genus { Genus::DontCare };
    Status status { Status::Active };
    Access access { Access::Public };

    QString title;
    QString fullTitle;
    QString url;
    QString since;
    QString deprecatedSince;
    QString brief;

    QList<ContentBlock> body;
    QList<SectionIR> summarySections;
    QList<SectionIR> detailSections;

    std::optional<QmlTypeData> qmlTypeData;
    std::optional<CollectionData> collectionData;
    std::optional<CppReferenceData> cppReferenceData;
    NavigationData navigationData;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_PAGEMETADATA_H
