// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SECTIONS_H
#define SECTIONS_H

#include "node.h"

QT_BEGIN_NAMESPACE

class Aggregate;

typedef std::pair<const QmlTypeNode *, NodeVector> ClassNodes;
typedef QList<ClassNodes> ClassNodesList;

class Section
{
public:
    enum Style { Summary, Details, AllMembers, Accessors };

public:
    Section(
        QString title, QString singular, QString plural,
        QString divclass, Style style
    ) : m_title{title}, m_singular{singular}, m_plural{plural},
        m_divClass{divclass}, m_style{style}
    {}

    void insert(Node *node);
    bool insertReimplementedMember(Node *node);

    void appendMember(Node *node) { m_members.append(node); }

    void reduce();
    [[nodiscard]] bool isEmpty() const
    {
        return (m_members.isEmpty() && m_inheritedMembers.isEmpty()
                && m_reimplementedMemberMap.isEmpty() && m_classNodesList.isEmpty());
    }

    [[nodiscard]] Style style() const { return m_style; }
    [[nodiscard]] const QString &title() const { return m_title; }
    [[nodiscard]] const QString &divClass() const { return m_divClass; }
    [[nodiscard]] const QString &singular() const { return m_singular; }
    [[nodiscard]] const QString &plural() const { return m_plural; }
    [[nodiscard]] const NodeVector &members() const { return m_members; }
    [[nodiscard]] const NodeVector &reimplementedMembers() const { return m_reimplementedMembers; }
    [[nodiscard]] const QList<std::pair<const Aggregate *, int>> &inheritedMembers() const
    {
        return m_inheritedMembers;
    }
    ClassNodesList &classNodesList() { return m_classNodesList; }
    [[nodiscard]] const ClassNodesList &classNodesList() const { return m_classNodesList; }
    [[nodiscard]] const NodeVector &obsoleteMembers() const { return m_obsoleteMembers; }
    void appendMembers(const NodeVector &nv) { m_members.append(nv); }
    [[nodiscard]] const Aggregate *aggregate() const { return m_aggregate; }
    void setAggregate(const Aggregate *t) { m_aggregate = t; }

private:
    QString m_title {};
    QString m_singular {};
    QString m_plural {};
    QString m_divClass {};
    Style m_style {};

    const Aggregate *m_aggregate { nullptr };
    NodeVector m_members {};
    NodeVector m_obsoleteMembers {};
    NodeVector m_reimplementedMembers {};
    QList<std::pair<const Aggregate *, int>> m_inheritedMembers {};
    ClassNodesList m_classNodesList {};

    QMultiMap<QString, Node *> m_reimplementedMemberMap {};
};

typedef QList<Section> SectionVector;
typedef QList<const Section *> SectionPtrVector;

class Sections
{
    Q_DISABLE_COPY_MOVE(Sections)

public:
    enum VectorIndex {
        PublicTypes = 0,
        DetailsMemberTypes = 0,
        SinceNamespaces = 0,
        StdNamespaces = 0,
        QmlEnumTypes = 0,
        QmlProperties = 1,
        Properties = 1,
        DetailsProperties = 1,
        SinceClasses = 1,
        StdClasses = 1,
        QmlAttachedProperties = 2,
        PublicFunctions = 2,
        DetailsMemberFunctions = 2,
        SinceMemberFunctions = 2,
        StdTypes = 2,
        QmlSignals = 3,
        PublicSlots = 3,
        DetailsMemberVariables = 3,
        SinceNamespaceFunctions = 3,
        StdVariables = 3,
        QmlSignalHandlers = 4,
        Signals = 4,
        SinceGlobalFunctions = 4,
        DetailsRelatedNonmembers = 4,
        StdStaticVariables = 4,
        QmlAttachedSignals = 5,
        PublicVariables = 5,
        SinceMacros = 5,
        DetailsMacros = 5,
        StdFunctions = 5,
        QmlMethods = 6,
        StaticPublicMembers = 6,
        SinceEnumTypes = 6,
        StdMacros = 6,
        QmlAttachedMethods = 7,
        SinceEnumValues = 7,
        ProtectedTypes = 7,
        SinceTypeAliases = 8,
        ProtectedFunctions = 8,
        SinceProperties = 9,
        ProtectedSlots = 9,
        SinceVariables = 10,
        ProtectedVariables = 10,
        SinceConcepts = 11,
        StaticProtectedMembers = 11,
        SinceQmlTypes = 12,
        PrivateTypes = 12,
        SinceQmlEnumTypes = 13,
        SinceQmlProperties = 14,
        PrivateFunctions = 13,
        SinceQmlSignals = 15,
        PrivateSlots = 14,
        SinceQmlSignalHandlers = 16,
        PrivateVariables = 15,
        StaticPrivateMembers = 16,
        SinceQmlMethods = 17,
        RelatedNonmembers = 17,
        Macros = 18
    };

    explicit Sections(const Aggregate *aggregate);
    explicit Sections(const NodeMultiMap &nsmap);

    [[nodiscard]] bool hasObsoleteMembers(SectionPtrVector *summary_spv,
                                          SectionPtrVector *details_spv) const;

    SectionVector &summarySections() { return m_summarySections; }
    SectionVector &detailsSections() { return m_detailsSections; }
    Section &allMembersSection() { return m_allMembers; }
    SectionVector &sinceSections() { return m_sinceSections; }

    [[nodiscard]] const SectionVector &summarySections() const { return m_summarySections; }
    [[nodiscard]] const SectionVector &detailsSections() const { return m_detailsSections; }
    [[nodiscard]] const Section &allMembersSection() const { return m_allMembers; }
    [[nodiscard]] const SectionVector &sinceSections() const { return m_sinceSections; }

    [[nodiscard]] const Aggregate *aggregate() const { return m_aggregate; }

private:
    void stdRefPageSwitch(SectionVector &v, Node *n);
    void distributeNodeInSummaryVector(SectionVector &sv, Node *n);
    void distributeNodeInDetailsVector(SectionVector &dv, Node *n);
    void distributeQmlNodeInDetailsVector(SectionVector &dv, Node *n);
    void distributeQmlNodeInSummaryVector(SectionVector &sv, Node *n, bool sharing = false);
    void initAggregate(SectionVector &v, const Aggregate *aggregate);
    void buildStdRefPageSections();
    void buildStdCppClassRefPageSections();
    void buildStdQmlTypeRefPageSections();
    void reduce(SectionVector &v);

private:
    const Aggregate *m_aggregate { nullptr };
    SectionVector m_summarySections;
    SectionVector m_detailsSections;
    Section m_allMembers { {}, "member", "members", {}, Section::AllMembers };
    SectionVector m_sinceSections;
};

QT_END_NAMESPACE

#endif
