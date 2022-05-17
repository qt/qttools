// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef AGGREGATE_H
#define AGGREGATE_H

#include "pagenode.h"

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

    void addIncludeFile(const QString &includeFile);
    void setIncludeFiles(const QStringList &includeFiles);
    [[nodiscard]] const QStringList &includeFiles() const { return m_includeFiles; }

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
    QStringList m_includeFiles {};
    NodeList m_enumChildren {};
    NodeMultiMap m_nonfunctionMap {};
    NodeList m_nonfunctionList {};
};

QT_END_NAMESPACE

#endif // AGGREGATE_H
