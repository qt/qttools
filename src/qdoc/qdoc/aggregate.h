// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef AGGREGATE_H
#define AGGREGATE_H

#include "pagenode.h"

#include <optional>

#include <QtCore/qglobal.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class FunctionNode;
class QmlTypeNode;
class QmlPropertyNode;

class Aggregate : public PageNode
{
public:
    [[nodiscard]] Node *findChildNode(const QString &name, Node::Genus genus,
                                      int findFlags = 0) const;
    Node *findNonfunctionChild(const QString &name, bool (Node::*)() const);
    void findChildren(const QString &name, NodeVector &nodes) const;
    FunctionNode *findFunctionChild(const QString &name, const Parameters &parameters);
    FunctionNode *findFunctionChild(const FunctionNode *clone);

    void normalizeOverloads();
    void markUndocumentedChildrenInternal();

    [[nodiscard]] bool isAggregate() const override { return true; }
    [[nodiscard]] const EnumNode *findEnumNodeForValue(const QString &enumValue) const;

    [[nodiscard]] qsizetype count() const { return m_children.size(); }
    [[nodiscard]] const NodeList &childNodes() const { return m_children; }
    const NodeList &nonfunctionList();
    [[nodiscard]] NodeList::ConstIterator constBegin() const { return m_children.constBegin(); }
    [[nodiscard]] NodeList::ConstIterator constEnd() const { return m_children.constEnd(); }

    inline void setIncludeFile(const QString& include) { m_includeFile.emplace(include); }
    // REMARK: Albeit not enforced at the API boundaries,
    // downstream-user can assume that if there is a QString that was
    // set, then that string is not empty.
    [[nodiscard]] inline const std::optional<QString>& includeFile() const { return m_includeFile; }

    [[nodiscard]] QmlPropertyNode *hasQmlProperty(const QString &) const;
    [[nodiscard]] QmlPropertyNode *hasQmlProperty(const QString &, bool attached) const;
    virtual QmlTypeNode *qmlBaseNode() const { return nullptr; }
    void addChildByTitle(Node *child, const QString &title);
    void addChild(Node *child);
    void adoptChild(Node *child);
    void setOutputSubdirectory(const QString &t) override;

    FunctionMap &functionMap() { return m_functionMap; }
    void findAllFunctions(NodeMapMap &functionIndex);
    void findAllNamespaces(NodeMultiMap &namespaces);
    void findAllAttributions(NodeMultiMap &attributions);
    [[nodiscard]] bool hasObsoleteMembers() const;
    void findAllObsoleteThings();
    void findAllClasses();
    void findAllSince();
    void resolveQmlInheritance();
    bool hasOverloads(const FunctionNode *fn) const;
    void appendToRelatedByProxy(const NodeList &t) { m_relatedByProxy.append(t); }
    NodeList &relatedByProxy() { return m_relatedByProxy; }
    [[nodiscard]] QString typeWord(bool cap) const;

protected:
    Aggregate(NodeType type, Aggregate *parent, const QString &name)
        : PageNode(type, parent, name) {}
    ~Aggregate() override;

private:
    friend class Node;
    void addFunction(FunctionNode *fn);
    void adoptFunction(FunctionNode *fn, Aggregate *firstParent);
    static bool isSameSignature(const FunctionNode *f1, const FunctionNode *f2);
    void dropNonRelatedMembers();

protected:
    NodeList m_children {};
    NodeList m_relatedByProxy {};
    FunctionMap m_functionMap {};

private:
    // REMARK: The member indicates the name of a file where the
    // aggregate can be found from, for example, an header file that
    // declares a class.
    // For aggregates such as classes we expect this to always be set
    // to a non-empty string after the code-parsing phase.
    // Indeed, currently, by default, QDoc always generates such a
    // string using the name of the aggregate if no include file can
    // be propagated from some of the parents.
    //
    // Nonetheless, we are still forced to make this an optional, as
    // this will not be true for all Aggregates.
    //
    // For example, for namespaces, we don't seem to set an include
    // file and indeed doing so wouldn't be particularly meaningful.
    //
    // It is possible to assume in later code, especially the
    // generation phase, that at least some classes of aggregates
    // always have a value set here but we should, for the moment,
    // still check for the possibility of something not to be there,
    // or warn if we decide to ignore that, to be compliant with the
    // current interface, whose change would require deep changes to
    // QDoc internal structures.
    std::optional<QString> m_includeFile{};
    NodeList m_enumChildren {};
    NodeMultiMap m_nonfunctionMap {};
    NodeList m_nonfunctionList {};
};

QT_END_NAMESPACE

#endif // AGGREGATE_H
