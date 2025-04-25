// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOCDATABASE_H
#define QDOCDATABASE_H

#include "config.h"
#include "examplenode.h"
#include "genustypes.h"
#include "propertynode.h"
#include "text.h"
#include "tree.h"

#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

typedef QMultiMap<Text, const Node *> TextToNodeMap;

class Atom;
class FunctionNode;
class Generator;
class QDocDatabase;

enum FindFlag {
    SearchBaseClasses = 0x1,
    SearchEnumValues = 0x2,
    TypesOnly = 0x4,
    IgnoreModules = 0x8
};

class QDocForest
{
private:
    friend class QDocDatabase;
    explicit QDocForest(QDocDatabase *qdb) : m_qdb(qdb), m_primaryTree(nullptr), m_currentIndex(0)
    {
    }
    ~QDocForest();

    Tree *firstTree();
    Tree *nextTree();
    Tree *primaryTree() { return m_primaryTree; }
    Tree *findTree(const QString &t) { return m_forest.value(t); }
    QStringList keys() { return m_forest.keys(); }
    NamespaceNode *primaryTreeRoot() { return (m_primaryTree ? m_primaryTree->root() : nullptr); }
    bool isEmpty() { return searchOrder().isEmpty(); }
    bool done() { return (m_currentIndex >= searchOrder().size()); }
    const QList<Tree *> &searchOrder();
    const QList<Tree *> &indexSearchOrder();
    void setSearchOrder(const QStringList &t);
    bool isLoaded(const QString &fn)
    {
        return std::any_of(searchOrder().constBegin(), searchOrder().constEnd(),
                           [fn](Tree *tree) { return fn == tree->indexFileName(); });
    }

    const Node *findNode(const QStringList &path, const Node *relative, int findFlags,
                         Genus genus)
    {
        for (const auto *tree : searchOrder()) {
            const Node *n = tree->findNode(path, relative, findFlags, genus);
            if (n)
                return n;
            relative = nullptr;
        }
        return nullptr;
    }

    Node *findNodeByNameAndType(const QStringList &path, bool (Node::*isMatch)() const)
    {
        for (const auto *tree : searchOrder()) {
            Node *n = tree->findNodeByNameAndType(path, isMatch);
            if (n)
                return n;
        }
        return nullptr;
    }

    ClassNode *findClassNode(const QStringList &path)
    {
        for (const auto *tree : searchOrder()) {
            ClassNode *n = tree->findClassNode(path);
            if (n)
                return n;
        }
        return nullptr;
    }

    Node *findNodeForInclude(const QStringList &path)
    {
        for (const auto *tree : searchOrder()) {
            Node *n = tree->findNodeForInclude(path);
            if (n)
                return n;
        }
        return nullptr;
    }

    const FunctionNode *findFunctionNode(const QStringList &path, const Parameters &parameters,
                                         const Node *relative, Genus genus);
    const Node *findNodeForTarget(QStringList &targetPath, const Node *relative, Genus genus,
                                  QString &ref);

    const Node *findTypeNode(const QStringList &path, const Node *relative, Genus genus)
    {
        int flags = SearchBaseClasses | SearchEnumValues | TypesOnly;
        if (relative && genus == Genus::DontCare && relative->genus() != Genus::DOC)
            genus = relative->genus();
        for (const auto *tree : searchOrder()) {
            const Node *n = tree->findNode(path, relative, flags, genus);
            if (n)
                return n;
            relative = nullptr;
        }
        return nullptr;
    }

    const PageNode *findPageNodeByTitle(const QString &title)
    {
        for (const auto *tree : searchOrder()) {
            const PageNode *n = tree->findPageNodeByTitle(title);
            if (n)
                return n;
        }
        return nullptr;
    }

    const CollectionNode *getCollectionNode(const QString &name, NodeType type)
    {
        for (auto *tree : searchOrder()) {
            const CollectionNode *cn = tree->getCollection(name, type);
            if (cn)
                return cn;
        }
        return nullptr;
    }

    QmlTypeNode *lookupQmlType(const QString &name)
    {
        for (const auto *tree : searchOrder()) {
            QmlTypeNode *qcn = tree->lookupQmlType(name);
            if (qcn)
                return qcn;
        }
        return nullptr;
    }

    void clearSearchOrder() { m_searchOrder.clear(); }
    void newPrimaryTree(const QString &module);
    void setPrimaryTree(const QString &t);
    NamespaceNode *newIndexTree(const QString &module);

private:
    QDocDatabase *m_qdb;
    Tree *m_primaryTree;
    int m_currentIndex;
    QMap<QString, Tree *> m_forest;
    QList<Tree *> m_searchOrder;
    QList<Tree *> m_indexSearchOrder;
    QList<QString> m_moduleNames;
};

class QDocDatabase
{
public:
    static QDocDatabase *qdocDB();
    static void destroyQdocDB();
    ~QDocDatabase() = default;

    using FindFunctionPtr = void (QDocDatabase::*)(Aggregate *);

    Tree *findTree(const QString &t) { return m_forest.findTree(t); }

    const CNMap &groups() { return primaryTree()->groups(); }
    const CNMap &modules() { return primaryTree()->modules(); }
    const CNMap &qmlModules() { return primaryTree()->qmlModules(); }

    CollectionNode *addGroup(const QString &name) { return primaryTree()->addGroup(name); }
    CollectionNode *addModule(const QString &name) { return primaryTree()->addModule(name); }
    CollectionNode *addQmlModule(const QString &name) { return primaryTree()->addQmlModule(name); }

    CollectionNode *addToGroup(const QString &name, Node *node)
    {
        return primaryTree()->addToGroup(name, node);
    }
    CollectionNode *addToModule(const QString &name, Node *node)
    {
        return primaryTree()->addToModule(name, node);
    }
    CollectionNode *addToQmlModule(const QString &name, Node *node)
    {
        return primaryTree()->addToQmlModule(name, node);
    }

    void addExampleNode(ExampleNode *n) { primaryTree()->addExampleNode(n); }
    ExampleNodeMap &exampleNodeMap() { return primaryTree()->exampleNodeMap(); }

    QmlTypeNode *findQmlType(const QString &name)
    {
        return m_forest.lookupQmlType(name);
    }
    QmlTypeNode *findQmlType(const QString &qmid, const QString &name);
    QmlTypeNode *findQmlType(const ImportRec &import, const QString &name);
    QmlTypeNode *findQmlTypeInPrimaryTree(const QString &qmid, const QString &name);

    static NodeMultiMap &obsoleteClasses() { return s_obsoleteClasses; }
    static NodeMultiMap &obsoleteQmlTypes() { return s_obsoleteQmlTypes; }
    static NodeMultiMap &classesWithObsoleteMembers() { return s_classesWithObsoleteMembers; }
    static NodeMultiMap &qmlTypesWithObsoleteMembers() { return s_qmlTypesWithObsoleteMembers; }
    static NodeMultiMap &cppClasses() { return s_cppClasses; }
    static NodeMultiMap &qmlBasicTypes() { return s_qmlBasicTypes; }
    static NodeMultiMap &qmlTypes() { return s_qmlTypes; }
    static NodeMultiMap &examples() { return s_examples; }
    static NodeMultiMapMap &newClassMaps() { return s_newClassMaps; }
    static NodeMultiMapMap &newQmlTypeMaps() { return s_newQmlTypeMaps; }
    static NodeMultiMapMap &newEnumValueMaps() { return s_newEnumValueMaps; }
    static NodeMultiMapMap &newSinceMaps() { return s_newSinceMaps; }

private:
    void findAllClasses(Aggregate *node) { node->findAllClasses(); }
    void findAllFunctions(Aggregate *node) { node->findAllFunctions(m_functionIndex); }
    void findAllAttributions(Aggregate *node) { node->findAllAttributions(m_attributions); }
    void findAllLegaleseTexts(Aggregate *node);
    void findAllObsoleteThings(Aggregate *node) { node->findAllObsoleteThings(); }
    void findAllSince(Aggregate *node) { node->findAllSince(); }

public:
    /*******************************************************************
     special collection access functions
    ********************************************************************/
    NodeMultiMap &getCppClasses();
    NodeMultiMap &getObsoleteClasses();
    NodeMultiMap &getClassesWithObsoleteMembers();
    NodeMultiMap &getObsoleteQmlTypes();
    NodeMultiMap &getQmlTypesWithObsoleteMembers();
    NodeMultiMap &getNamespaces();
    NodeMultiMap &getQmlValueTypes();
    NodeMultiMap &getQmlTypes();
    NodeMultiMap &getExamples();
    NodeMultiMap &getAttributions();
    NodeMapMap &getFunctionIndex();
    TextToNodeMap &getLegaleseTexts();
    const NodeMultiMap &getClassMap(const QString &key);
    const NodeMultiMap &getQmlTypeMap(const QString &key);
    const NodeMultiMap &getSinceMap(const QString &key);

    /*******************************************************************
      Many of these will be either eliminated or replaced.
    ********************************************************************/
    void resolveStuff();
    void insertTarget(const QString &name, const QString &title, TargetRec::TargetType type,
                      Node *node, int priority)
    {
        primaryTree()->insertTarget(name, title, type, node, priority);
    }

    /*******************************************************************
      The functions declared below are called for the current tree only.
    ********************************************************************/
    Aggregate *findRelatesNode(const QStringList &path)
    {
        return primaryTree()->findRelatesNode(path);
    }
    /*******************************************************************/

    /*****************************************************************************
      This function can handle parameters enclosed in '[' ']' (domain and genus).
    ******************************************************************************/
    const Node *findNodeForAtom(const Atom *atom, const Node *relative, QString &ref,
                                Genus genus = Genus::DontCare);
    /*******************************************************************/

    /*******************************************************************
      The functions declared below are called for all trees.
    ********************************************************************/
    ClassNode *findClassNode(const QStringList &path) { return m_forest.findClassNode(path); }
    Node *findNodeForInclude(const QStringList &path) { return m_forest.findNodeForInclude(path); }
    const FunctionNode *findFunctionNode(const QString &target, const Node *relative,
                                         Genus genus);
    const Node *findTypeNode(const QString &type, const Node *relative, Genus genus);
    const Node *findNodeForTarget(const QString &target, const Node *relative);
    const PageNode *findPageNodeByTitle(const QString &title)
    {
        return m_forest.findPageNodeByTitle(title);
    }
    Node *findNodeByNameAndType(const QStringList &path, bool (Node::*isMatch)() const)
    {
        return m_forest.findNodeByNameAndType(path, isMatch);
    }
    const CollectionNode *getCollectionNode(const QString &name, NodeType type)
    {
        return m_forest.getCollectionNode(name, type);
    }
    const CollectionNode *getModuleNode(const Node *relative);

    FunctionNode *findFunctionNodeForTag(const QString &tag)
    {
        return primaryTree()->findFunctionNodeForTag(tag);
    }
    FunctionNode *findMacroNode(const QString &t) { return primaryTree()->findMacroNode(t); }

    QStringList groupNamesForNode(Node *node);

private:
    const Node *findNodeForTarget(QStringList &targetPath, const Node *relative, Genus genus,
                                  QString &ref)
    {
        return m_forest.findNodeForTarget(targetPath, relative, genus, ref);
    }
    const FunctionNode *findFunctionNode(const QStringList &path, const Parameters &parameters,
                                         const Node *relative, Genus genus)
    {
        return m_forest.findFunctionNode(path, parameters, relative, genus);
    }

    /*******************************************************************/
public:
    void addPropertyFunction(PropertyNode *property, const QString &funcName,
                             PropertyNode::FunctionRole funcRole)
    {
        primaryTree()->addPropertyFunction(property, funcName, funcRole);
    }

    void setVersion(const QString &v) { m_version = v; }
    [[nodiscard]] QString version() const { return m_version; }

    void readIndexes(const QStringList &indexFiles);
    void generateIndex(const QString &fileName, const QString &url, const QString &title);

    void processForest();

    NamespaceNode *primaryTreeRoot() { return m_forest.primaryTreeRoot(); }
    void newPrimaryTree(const QString &module) { m_forest.newPrimaryTree(module); }
    void setPrimaryTree(const QString &t) { m_forest.setPrimaryTree(t); }
    NamespaceNode *newIndexTree(const QString &module) { return m_forest.newIndexTree(module); }
    const QList<Tree *> &searchOrder() { return m_forest.searchOrder(); }
    void setLocalSearch() { m_forest.m_searchOrder = QList<Tree *>(1, primaryTree()); }
    void setSearchOrder(const QList<Tree *> &searchOrder) { m_forest.m_searchOrder = searchOrder; }
    void setSearchOrder(QStringList &t) { m_forest.setSearchOrder(t); }
    void mergeCollections(NodeType type, CNMap &cnm, const Node *relative);
    void mergeCollections(CollectionNode *c);
    void clearSearchOrder() { m_forest.clearSearchOrder(); }
    QStringList keys() { return m_forest.keys(); }
    void resolveNamespaces();
    void resolveProxies();
    void resolveBaseClasses();
    void updateNavigation();

private:
    friend class Tree;

    void processForest(FindFunctionPtr func);
    bool isLoaded(const QString &t) { return m_forest.isLoaded(t); }
    static void initializeDB();

private:
    QDocDatabase();
    QDocDatabase(QDocDatabase const &) : m_forest(this) { }
    QDocDatabase &operator=(QDocDatabase const &);

public:
    Tree *primaryTree() { return m_forest.primaryTree(); }

private:
    static QDocDatabase *s_qdocDB;
    static NodeMap s_typeNodeMap;
    static NodeMultiMap s_obsoleteClasses;
    static NodeMultiMap s_classesWithObsoleteMembers;
    static NodeMultiMap s_obsoleteQmlTypes;
    static NodeMultiMap s_qmlTypesWithObsoleteMembers;
    static NodeMultiMap s_cppClasses;
    static NodeMultiMap s_qmlBasicTypes;
    static NodeMultiMap s_qmlTypes;
    static NodeMultiMap s_examples;
    static NodeMultiMapMap s_newClassMaps;
    static NodeMultiMapMap s_newQmlTypeMaps;
    static NodeMultiMapMap s_newEnumValueMaps;
    static NodeMultiMapMap s_newSinceMaps;

    QString m_version {};
    QDocForest m_forest;

    NodeMultiMap m_namespaceIndex {};
    NodeMultiMap m_attributions {};
    NodeMapMap m_functionIndex {};
    TextToNodeMap m_legaleseTexts {};
    QMultiHash<Tree*, FindFunctionPtr> m_completedFindFunctions {};
};

QT_END_NAMESPACE

#endif
