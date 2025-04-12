// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TREE_H
#define TREE_H

#include "examplenode.h"
#include "genustypes.h"
#include "namespacenode.h"
#include "node.h"
#include "propertynode.h"
#include "proxynode.h"
#include "qmltypenode.h"

#include <QtCore/qstack.h>

#include <utility>

QT_BEGIN_NAMESPACE

class CollectionNode;
class FunctionNode;
class QDocDatabase;

struct TargetRec
{
public:
    enum TargetType { Unknown, Target, Keyword, Contents, ContentsKeyword };

    TargetRec(QString name, TargetRec::TargetType type, Node *node, int priority)
        : m_node(node),
          m_ref(std::move(name)),
          m_type(type == ContentsKeyword ? Keyword : type),
          m_priority(priority)
    {
        // Discard the dedicated ref for keywords - they always
        // link to the top of the QDoc comment they appear in
        if (type == Keyword)
            m_ref.clear();
    }

    [[nodiscard]] bool isEmpty() const { return m_ref.isEmpty(); }
    [[nodiscard]] Genus genus() const { return (m_node ? m_node->genus() : Genus::DontCare); }

    Node *m_node { nullptr };
    QString m_ref {};
    TargetType m_type {};
    int m_priority {};
};

typedef QMultiMap<QString, TargetRec *> TargetMap;
typedef QMultiMap<QString, PageNode *> PageNodeMultiMap;
typedef QMap<QString, QmlTypeNode *> QmlTypeMap;
typedef QMultiMap<QString, const ExampleNode *> ExampleNodeMap;

class Tree
{
    friend class QDocForest;
    friend class QDocDatabase;

private: // Note the constructor and destructor are private.
    typedef QMap<PropertyNode::FunctionRole, QString> RoleMap;
    typedef QMap<PropertyNode *, RoleMap> PropertyMap;

    Tree(const QString &camelCaseModuleName, QDocDatabase *qdb);
    ~Tree();

public: // Of necessity, a few public functions remain.
    [[nodiscard]] Node *findNodeByNameAndType(const QStringList &path,
                                              bool (Node::*isMatch)() const) const;

    [[nodiscard]] const QString &camelCaseModuleName() const { return m_camelCaseModuleName; }
    [[nodiscard]] const QString &physicalModuleName() const { return m_physicalModuleName; }
    [[nodiscard]] const QString &indexFileName() const { return m_indexFileName; }
    [[nodiscard]] const QString &indexTitle() const { return m_indexTitle; }
    void setIndexTitle(const QString &t) { m_indexTitle = t; }
    NodeList &proxies() { return m_proxies; }
    void appendProxy(ProxyNode *t) { m_proxies.append(t); }
    void addToDontDocumentMap(QString &arg);
    void markDontDocumentNodes();
    static QString refForAtom(const Atom *atom);

private: // The rest of the class is private.
    Aggregate *findAggregate(const QString &name);
    [[nodiscard]] Node *findNodeForInclude(const QStringList &path) const;
    ClassNode *findClassNode(const QStringList &path, const Node *start = nullptr) const;
    [[nodiscard]] NamespaceNode *findNamespaceNode(const QStringList &path) const;
    const FunctionNode *findFunctionNode(const QStringList &path, const Parameters &parameters,
                                         const Node *relative, Genus genus) const;
    Node *findNodeRecursive(const QStringList &path, int pathIndex, const Node *start,
                            bool (Node::*)() const) const;
    const Node *findNodeForTarget(const QStringList &path, const QString &target, const Node *node,
                                  int flags, Genus genus, QString &ref,
                                  TargetRec::TargetType *targetType = nullptr) const;
    const Node *matchPathAndTarget(const QStringList &path, int idx, const QString &target,
                                   const Node *node, int flags, Genus genus,
                                   QString &ref) const;

    const Node *findNode(const QStringList &path, const Node *relative, int flags,
                         Genus genus) const;

    Aggregate *findRelatesNode(const QStringList &path);
    const Node *findEnumNode(const Node *node, const Node *aggregate, const QStringList &path, int offset) const;
    QString getRef(const QString &target, const Node *node) const;
    void insertTarget(const QString &name, const QString &title, TargetRec::TargetType type,
                      Node *node, int priority);
    void resolveTargets(Aggregate *root);
    void addToPageNodeByTitleMap(Node *node);
    void populateTocSectionTargetMap(Node *node);
    void addKeywordsToTargetMaps(Node *node);
    void addTargetsToTargetMap(Node *node);

    const TargetRec *findUnambiguousTarget(const QString &target, Genus genus) const;
    [[nodiscard]] const PageNode *findPageNodeByTitle(const QString &title) const;

    void addPropertyFunction(PropertyNode *property, const QString &funcName,
                             PropertyNode::FunctionRole funcRole);
    void resolveBaseClasses(Aggregate *n);
    void resolvePropertyOverriddenFromPtrs(Aggregate *n);
    void resolveProperties();
    void resolveCppToQmlLinks();
    void resolveSince(Aggregate &aggregate);
    void resolveEnumValueSince(EnumNode &en);
    void removePrivateAndInternalBases(NamespaceNode *rootNode);
    NamespaceNode *root() { return &m_root; }
    [[nodiscard]] const NamespaceNode *root() const { return &m_root; }

    ClassList allBaseClasses(const ClassNode *classe) const;

    CNMap *getCollectionMap(NodeType type);
    [[nodiscard]] const CNMap &groups() const { return m_groups; }
    [[nodiscard]] const CNMap &modules() const { return m_modules; }
    [[nodiscard]] const CNMap &qmlModules() const { return m_qmlModules; }

    CollectionNode *getCollection(const QString &name, NodeType type);
    CollectionNode *findCollection(const QString &name, NodeType type);

    CollectionNode *findGroup(const QString &name) { return findCollection(name, NodeType::Group); }
    CollectionNode *findModule(const QString &name) { return findCollection(name, NodeType::Module); }
    CollectionNode *findQmlModule(const QString &name)
    {
        return findCollection(name, NodeType::QmlModule);
    }

    CollectionNode *addGroup(const QString &name) { return findGroup(name); }
    CollectionNode *addModule(const QString &name) { return findModule(name); }
    CollectionNode *addQmlModule(const QString &name) { return findQmlModule(name); }

    CollectionNode *addToGroup(const QString &name, Node *node);
    CollectionNode *addToModule(const QString &name, Node *node);
    CollectionNode *addToQmlModule(const QString &name, Node *node);

    [[nodiscard]] QmlTypeNode *lookupQmlType(const QString &name) const
    {
        return m_qmlTypeMap.value(name);
    }
    void insertQmlType(const QString &key, QmlTypeNode *n);
    void addExampleNode(ExampleNode *n) { m_exampleNodeMap.insert(n->title(), n); }
    ExampleNodeMap &exampleNodeMap() { return m_exampleNodeMap; }
    void setIndexFileName(const QString &t) { m_indexFileName = t; }

    FunctionNode *findFunctionNodeForTag(const QString &tag, Aggregate *parent = nullptr);
    FunctionNode *findMacroNode(const QString &t, const Aggregate *parent = nullptr);

private:
    QString m_camelCaseModuleName {};
    QString m_physicalModuleName {};
    QString m_indexFileName {};
    QString m_indexTitle {};
    QDocDatabase *m_qdb { nullptr };
    NamespaceNode m_root;
    PropertyMap m_unresolvedPropertyMap {};
    PageNodeMultiMap m_pageNodesByTitle {};
    TargetMap m_nodesByTargetRef {};
    TargetMap m_nodesByTargetTitle {};
    CNMap m_groups {};
    CNMap m_modules {};
    CNMap m_qmlModules {};
    QmlTypeMap m_qmlTypeMap {};
    ExampleNodeMap m_exampleNodeMap {};
    NodeList m_proxies {};
    NodeMap m_dontDocumentMap {};
};

QT_END_NAMESPACE

#endif
