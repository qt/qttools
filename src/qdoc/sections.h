// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SECTIONS_H
#define SECTIONS_H

#include "node.h"

QT_BEGIN_NAMESPACE

class Aggregate;

typedef QMultiMap<QString, Node *> MemberMap; // the string is the member signature
typedef std::pair<const QmlTypeNode *, MemberMap> ClassMap; // the node is the QML type
typedef QList<ClassMap *> ClassMapList;

typedef std::pair<QStringList, NodeVector> KeysAndNodes;
typedef std::pair<const QmlTypeNode *, KeysAndNodes> ClassKeysNodes;
typedef QList<ClassKeysNodes *> ClassKeysNodesList;

class Section
{
public:
    enum Style { Summary, Details, AllMembers, Accessors };
    enum Status { Obsolete, Active };

public:
    Section() : m_style(Details), m_status(Active), m_aggregate(nullptr) { }
    Section(Style style, Status status) : m_style(style), m_status(status), m_aggregate(nullptr) {};
    ~Section();

    void init(const QString &title) { m_title = title; }
    void init(const QString &singular, const QString &plural)
    {
        m_singular = singular;
        m_plural = plural;
    }
    void init(const QString &title, const QString &singular, const QString &plural)
    {
        m_title = title;
        m_divClass.clear();
        m_singular = singular;
        m_plural = plural;
    }
    void init(const QString &title, const QString &divClass, const QString &singular,
              const QString &plural)
    {
        m_title = title;
        m_divClass = divClass;
        m_singular = singular;
        m_plural = plural;
    }

    void insert(Node *node);
    void insert(const QString &key, Node *node) { m_memberMap.insert(key, node); }
    bool insertReimplementedMember(Node *node);

    ClassMap *newClassMap(const Aggregate *aggregate);
    void add(ClassMap *classMap, Node *n);
    void appendMember(Node *node) { m_members.append(node); }

    void clear();
    void reduce();
    [[nodiscard]] bool isEmpty() const
    {
        return (m_memberMap.isEmpty() && m_inheritedMembers.isEmpty()
                && m_reimplementedMemberMap.isEmpty() && m_classMapList.isEmpty());
    }

    [[nodiscard]] Style style() const { return m_style; }
    [[nodiscard]] Status status() const { return m_status; }
    [[nodiscard]] const QString &title() const { return m_title; }
    [[nodiscard]] const QString &divClass() const { return m_divClass; }
    [[nodiscard]] const QString &singular() const { return m_singular; }
    [[nodiscard]] const QString &plural() const { return m_plural; }
    [[nodiscard]] const QStringList &keys() const { return m_keys; }
    [[nodiscard]] const QStringList &keys(Status t) const
    {
        return (t == Obsolete ? m_obsoleteKeys : m_keys);
    }
    [[nodiscard]] const NodeVector &members() const { return m_members; }
    [[nodiscard]] const NodeVector &reimplementedMembers() const { return m_reimplementedMembers; }
    [[nodiscard]] const QList<std::pair<Aggregate *, int>> &inheritedMembers() const
    {
        return m_inheritedMembers;
    }
    ClassKeysNodesList &classKeysNodesList() { return m_classKeysNodesList; }
    [[nodiscard]] const NodeVector &obsoleteMembers() const { return m_obsoleteMembers; }
    void appendMembers(const NodeVector &nv) { m_members.append(nv); }
    [[nodiscard]] const Aggregate *aggregate() const { return m_aggregate; }
    void setAggregate(Aggregate *t) { m_aggregate = t; }

private:
    QString sortName(const Node *node, const QString *name = nullptr);

private:
    Style m_style {};
    Status m_status {};
    QString m_title {};
    QString m_divClass {};
    QString m_singular {};
    QString m_plural {};

    Aggregate *m_aggregate { nullptr };
    QStringList m_keys {};
    QStringList m_obsoleteKeys {};
    NodeVector m_members {};
    NodeVector m_obsoleteMembers {};
    NodeVector m_reimplementedMembers {};
    QList<std::pair<Aggregate *, int>> m_inheritedMembers {};
    ClassKeysNodesList m_classKeysNodesList {};

    QMultiMap<QString, Node *> m_memberMap {};
    QMultiMap<QString, Node *> m_obsoleteMemberMap {};
    QMultiMap<QString, Node *> m_reimplementedMemberMap {};
    ClassMapList m_classMapList {};
};

typedef QList<Section> SectionVector;
typedef QList<const Section *> SectionPtrVector;

class Sections
{
public:
    enum VectorIndex {
        PublicTypes = 0,
        DetailsMemberTypes = 0,
        SinceNamespaces = 0,
        StdNamespaces = 0,
        QmlProperties = 0,
        Properties = 1,
        DetailsProperties = 1,
        SinceClasses = 1,
        StdClasses = 1,
        QmlAttachedProperties = 1,
        PublicFunctions = 2,
        DetailsMemberFunctions = 2,
        SinceMemberFunctions = 2,
        StdTypes = 2,
        QmlSignals = 2,
        PublicSlots = 3,
        DetailsMemberVariables = 3,
        SinceNamespaceFunctions = 3,
        StdVariables = 3,
        QmlSignalHandlers = 3,
        Signals = 4,
        SinceGlobalFunctions = 4,
        DetailsRelatedNonmembers = 4,
        StdStaticVariables = 4,
        QmlAttachedSignals = 4,
        PublicVariables = 5,
        SinceMacros = 5,
        DetailsMacros = 5,
        StdFunctions = 5,
        QmlMethods = 5,
        StaticPublicMembers = 6,
        SinceEnumTypes = 6,
        StdMacros = 6,
        QmlAttachedMethods = 6,
        ProtectedTypes = 7,
        SinceTypedefs = 7,
        SinceTypeAliases = 7,
        ProtectedFunctions = 8,
        SinceProperties = 8,
        ProtectedSlots = 9,
        SinceVariables = 9,
        ProtectedVariables = 10,
        SinceQmlTypes = 10,
        StaticProtectedMembers = 11,
        SinceQmlProperties = 11,
        PrivateTypes = 12,
        SinceQmlSignals = 12,
        PrivateFunctions = 13,
        SinceQmlSignalHandlers = 13,
        PrivateSlots = 14,
        SinceQmlMethods = 14,
        StaticPrivateMembers = 15,
        RelatedNonmembers = 16,
        Macros = 17
    };

    explicit Sections(Aggregate *aggregate);
    explicit Sections(const NodeMultiMap &nsmap);
    ~Sections();

    void initSections();
    void clear(SectionVector &v);
    void reduce(SectionVector &v);
    void buildStdRefPageSections();
    void buildStdCppClassRefPageSections();
    void buildStdQmlTypeRefPageSections();

    bool hasObsoleteMembers(SectionPtrVector *summary_spv, SectionPtrVector *details_spv) const;

    static Section &allMembersSection() { return s_allMembers[0]; }
    SectionVector &sinceSections() { return s_sinceSections; }
    SectionVector &stdSummarySections() { return s_stdSummarySections; }
    SectionVector &stdDetailsSections() { return s_stdDetailsSections; }
    SectionVector &stdCppClassSummarySections() { return s_stdCppClassSummarySections; }
    SectionVector &stdCppClassDetailsSections() { return s_stdCppClassDetailsSections; }
    SectionVector &stdQmlTypeSummarySections() { return s_stdQmlTypeSummarySections; }
    SectionVector &stdQmlTypeDetailsSections() { return s_stdQmlTypeDetailsSections; }

    [[nodiscard]] const SectionVector &stdSummarySections() const { return s_stdSummarySections; }
    [[nodiscard]] const SectionVector &stdDetailsSections() const { return s_stdDetailsSections; }
    [[nodiscard]] const SectionVector &stdCppClassSummarySections() const
    {
        return s_stdCppClassSummarySections;
    }
    [[nodiscard]] const SectionVector &stdCppClassDetailsSections() const
    {
        return s_stdCppClassDetailsSections;
    }
    [[nodiscard]] const SectionVector &stdQmlTypeSummarySections() const
    {
        return s_stdQmlTypeSummarySections;
    }
    [[nodiscard]] const SectionVector &stdQmlTypeDetailsSections() const
    {
        return s_stdQmlTypeDetailsSections;
    }

    [[nodiscard]] Aggregate *aggregate() const { return m_aggregate; }

private:
    void stdRefPageSwitch(SectionVector &v, Node *n, Node *t = nullptr);
    void distributeNodeInSummaryVector(SectionVector &sv, Node *n);
    void distributeNodeInDetailsVector(SectionVector &dv, Node *n);
    void distributeQmlNodeInDetailsVector(SectionVector &dv, Node *n);
    void distributeQmlNodeInSummaryVector(SectionVector &sv, Node *n, bool sharing = false);
    void initAggregate(SectionVector &v, Aggregate *aggregate);

private:
    Aggregate *m_aggregate { nullptr };

    static SectionVector s_stdSummarySections;
    static SectionVector s_stdDetailsSections;
    static SectionVector s_stdCppClassSummarySections;
    static SectionVector s_stdCppClassDetailsSections;
    static SectionVector s_stdQmlTypeSummarySections;
    static SectionVector s_stdQmlTypeDetailsSections;
    static SectionVector s_sinceSections;
    static SectionVector s_allMembers;
};

QT_END_NAMESPACE

#endif
