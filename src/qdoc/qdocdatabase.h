/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDOCDATABASE_H
#define QDOCDATABASE_H

#include "config.h"
#include "text.h"
#include "tree.h"

#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

typedef QMultiMap<Text, const Node *> TextToNodeMap;

class Atom;
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
    QDocForest(QDocDatabase *qdb) : qdb_(qdb), primaryTree_(nullptr), currentIndex_(0) {}
    ~QDocForest();

    NamespaceNode *firstRoot();
    NamespaceNode *nextRoot();
    Tree *firstTree();
    Tree *nextTree();
    Tree *primaryTree() { return primaryTree_; }
    Tree *findTree(const QString &t) { return forest_.value(t); }
    QStringList keys() { return forest_.keys(); }
    NamespaceNode *primaryTreeRoot() { return (primaryTree_ ? primaryTree_->root() : nullptr); }
    bool isEmpty() { return searchOrder().isEmpty(); }
    bool done() { return (currentIndex_ >= searchOrder().size()); }
    const QVector<Tree *> &searchOrder();
    const QVector<Tree *> &indexSearchOrder();
    void setSearchOrder(const QStringList &t);
    bool isLoaded(const QString &fn)
    {
        for (const auto *tree : searchOrder()) {
            if (fn == tree->indexFileName())
                return true;
        }
        return false;
    }

    const Node *findNode(const QStringList &path, const Node *relative, int findFlags,
                         Node::Genus genus)
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
                                         const Node *relative, Node::Genus genus);
    const Node *findNodeForTarget(QStringList &targetPath, const Node *relative, Node::Genus genus,
                                  QString &ref);

    const Node *findTypeNode(const QStringList &path, const Node *relative, Node::Genus genus)
    {
        int flags = SearchBaseClasses | SearchEnumValues | TypesOnly;
        if (relative && genus == Node::DontCare && relative->genus() != Node::DOC)
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

    const CollectionNode *getCollectionNode(const QString &name, Node::NodeType type)
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

    Aggregate *lookupQmlBasicType(const QString &name)
    {
        for (const auto *tree : searchOrder()) {
            Aggregate *a = tree->lookupQmlBasicType(name);
            if (a)
                return a;
        }
        return nullptr;
    }
    void clearSearchOrder() { searchOrder_.clear(); }
    void clearLinkCounts()
    {
        for (auto *tree : searchOrder())
            tree->clearLinkCount();
    }
    void printLinkCounts(const QString &project);
    QString getLinkCounts(QStringList &strings, QVector<int> &counts);
    void newPrimaryTree(const QString &module);
    void setPrimaryTree(const QString &t);
    NamespaceNode *newIndexTree(const QString &module);

private:
    QDocDatabase *qdb_;
    Tree *primaryTree_;
    int currentIndex_;
    QMap<QString, Tree *> forest_;
    QVector<Tree *> searchOrder_;
    QVector<Tree *> indexSearchOrder_;
    QVector<QString> moduleNames_;
};

class QDocDatabase
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::QDocDatabase)

public:
    static QDocDatabase *qdocDB();
    static void destroyQdocDB();
    ~QDocDatabase();

    Tree *findTree(const QString &t) { return forest_.findTree(t); }

    const CNMap &groups() { return primaryTree()->groups(); }
    const CNMap &modules() { return primaryTree()->modules(); }
    const CNMap &qmlModules() { return primaryTree()->qmlModules(); }
    const CNMap &jsModules() { return primaryTree()->jsModules(); }

    CollectionNode *addGroup(const QString &name) { return primaryTree()->addGroup(name); }
    CollectionNode *addModule(const QString &name) { return primaryTree()->addModule(name); }
    CollectionNode *addQmlModule(const QString &name) { return primaryTree()->addQmlModule(name); }
    CollectionNode *addJsModule(const QString &name) { return primaryTree()->addJsModule(name); }

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
    CollectionNode *addToJsModule(const QString &name, Node *node)
    {
        return primaryTree()->addToJsModule(name, node);
    }

    void addExampleNode(ExampleNode *n) { primaryTree()->addExampleNode(n); }
    ExampleNodeMap &exampleNodeMap() { return primaryTree()->exampleNodeMap(); }

    QmlTypeNode *findQmlType(const QString &name);
    QmlTypeNode *findQmlType(const QString &qmid, const QString &name);
    QmlTypeNode *findQmlType(const ImportRec &import, const QString &name);
    Aggregate *findQmlBasicType(const QString &qmid, const QString &name);

    static NodeMultiMap &obsoleteClasses() { return obsoleteClasses_; }
    static NodeMultiMap &obsoleteQmlTypes() { return obsoleteQmlTypes_; }
    static NodeMultiMap &classesWithObsoleteMembers() { return classesWithObsoleteMembers_; }
    static NodeMultiMap &qmlTypesWithObsoleteMembers() { return qmlTypesWithObsoleteMembers_; }
    static NodeMultiMap &cppClasses() { return cppClasses_; }
    static NodeMultiMap &qmlBasicTypes() { return qmlBasicTypes_; }
    static NodeMultiMap &qmlTypes() { return qmlTypes_; }
    static NodeMultiMap &examples() { return examples_; }
    static NodeMapMap &newClassMaps() { return newClassMaps_; }
    static NodeMapMap &newQmlTypeMaps() { return newQmlTypeMaps_; }
    static NodeMultiMapMap &newSinceMaps() { return newSinceMaps_; }

private:
    void findAllClasses(Aggregate *node) { node->findAllClasses(); }
    void findAllFunctions(Aggregate *node) { node->findAllFunctions(functionIndex_); }
    void findAllAttributions(Aggregate *node) { node->findAllAttributions(attributions_); }
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
    NodeMultiMap &getQmlBasicTypes();
    NodeMultiMap &getQmlTypes();
    NodeMultiMap &getExamples();
    NodeMultiMap &getAttributions();
    NodeMapMap &getFunctionIndex();
    TextToNodeMap &getLegaleseTexts();
    const NodeMap &getClassMap(const QString &key);
    const NodeMap &getQmlTypeMap(const QString &key);
    const NodeMap &getSinceMap(const QString &key);

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
    Node *findNodeInOpenNamespace(QStringList &path, bool (Node::*)() const);
    /*******************************************************************/

    /*****************************************************************************
      This function can handle parameters enclosed in '[' ']' (domanin and genus).
    ******************************************************************************/
    const Node *findNodeForAtom(const Atom *atom, const Node *relative, QString &ref);
    /*******************************************************************/

    /*******************************************************************
      The functions declared below are called for all trees.
    ********************************************************************/
    ClassNode *findClassNode(const QStringList &path) { return forest_.findClassNode(path); }
    Node *findNodeForInclude(const QStringList &path) { return forest_.findNodeForInclude(path); }
    const FunctionNode *findFunctionNode(const QString &target, const Node *relative,
                                         Node::Genus genus);
    const Node *findTypeNode(const QString &type, const Node *relative, Node::Genus genus);
    const Node *findNodeForTarget(const QString &target, const Node *relative);
    const PageNode *findPageNodeByTitle(const QString &title)
    {
        return forest_.findPageNodeByTitle(title);
    }
    Node *findNodeByNameAndType(const QStringList &path, bool (Node::*isMatch)() const)
    {
        return forest_.findNodeByNameAndType(path, isMatch);
    }
    const CollectionNode *getCollectionNode(const QString &name, Node::NodeType type)
    {
        return forest_.getCollectionNode(name, type);
    }
    FunctionNode *findFunctionNodeForTag(QString tag)
    {
        return primaryTree()->findFunctionNodeForTag(tag);
    }
    FunctionNode *findMacroNode(const QString &t) { return primaryTree()->findMacroNode(t); }

private:
    const Node *findNodeForTarget(QStringList &targetPath, const Node *relative, Node::Genus genus,
                                  QString &ref)
    {
        return forest_.findNodeForTarget(targetPath, relative, genus, ref);
    }
    const FunctionNode *findFunctionNode(const QStringList &path, const Parameters &parameters,
                                         const Node *relative, Node::Genus genus)
    {
        return forest_.findFunctionNode(path, parameters, relative, genus);
    }

    /*******************************************************************/
public:
    void addPropertyFunction(PropertyNode *property, const QString &funcName,
                             PropertyNode::FunctionRole funcRole)
    {
        primaryTree()->addPropertyFunction(property, funcName, funcRole);
    }

    void setVersion(const QString &v) { version_ = v; }
    QString version() const { return version_; }

    void generateTagFile(const QString &name, Generator *g);
    void readIndexes(const QStringList &indexFiles);
    void generateIndex(const QString &fileName, const QString &url, const QString &title,
                       Generator *g);

    void clearOpenNamespaces() { openNamespaces_.clear(); }
    void insertOpenNamespace(const QString &path) { openNamespaces_.insert(path); }
    void setShowInternal(bool value) { showInternal_ = value; }
    void setSingleExec(bool value) { singleExec_ = value; }
    void processForest();

    // Try to make this function private.
    QDocForest &forest() { return forest_; }
    NamespaceNode *primaryTreeRoot() { return forest_.primaryTreeRoot(); }
    void newPrimaryTree(const QString &module) { forest_.newPrimaryTree(module); }
    void setPrimaryTree(const QString &t) { forest_.setPrimaryTree(t); }
    NamespaceNode *newIndexTree(const QString &module) { return forest_.newIndexTree(module); }
    const QVector<Tree *> &searchOrder() { return forest_.searchOrder(); }
    void setLocalSearch() { forest_.searchOrder_ = QVector<Tree *>(1, primaryTree()); }
    void setSearchOrder(const QVector<Tree *> &searchOrder) { forest_.searchOrder_ = searchOrder; }
    void setSearchOrder(QStringList &t) { forest_.setSearchOrder(t); }
    void mergeCollections(Node::NodeType type, CNMap &cnm, const Node *relative);
    void mergeCollections(CollectionNode *c);
    void clearSearchOrder() { forest_.clearSearchOrder(); }
    void incrementLinkCount(const Node *t) { t->tree()->incrementLinkCount(); }
    void clearLinkCounts() { forest_.clearLinkCounts(); }
    void printLinkCounts(const QString &t) { forest_.printLinkCounts(t); }
    QString getLinkCounts(QStringList &strings, QVector<int> &counts)
    {
        return forest_.getLinkCounts(strings, counts);
    }
    QString getNewLinkTarget(const Node *locNode, const Node *t, const QString &fileName,
                             QString &text, bool broken = false)
    {
        return primaryTree()->getNewLinkTarget(locNode, t, fileName, text, broken);
    }
    TargetList *getTargetList(const QString &t) { return primaryTree()->getTargetList(t); }
    QStringList getTargetListKeys() { return primaryTree()->getTargetListKeys(); }
    QStringList keys() { return forest_.keys(); }
    void resolveNamespaces();
    void resolveProxies();
    void resolveBaseClasses();

private:
    friend class Tree;

    const Node *findNode(const QStringList &path, const Node *relative, int findFlags,
                         Node::Genus genus)
    {
        return forest_.findNode(path, relative, findFlags, genus);
    }
    void processForest(void (QDocDatabase::*)(Aggregate *));
    bool isLoaded(const QString &t) { return forest_.isLoaded(t); }
    static void initializeDB();

private:
    QDocDatabase();
    QDocDatabase(QDocDatabase const &) : showInternal_(false), forest_(this) {}
    QDocDatabase &operator=(QDocDatabase const &);

public:
    static bool debug;
    Tree *primaryTree() { return forest_.primaryTree(); }

private:
    static QDocDatabase *qdocDB_;
    static NodeMap typeNodeMap_;
    static NodeMultiMap obsoleteClasses_;
    static NodeMultiMap classesWithObsoleteMembers_;
    static NodeMultiMap obsoleteQmlTypes_;
    static NodeMultiMap qmlTypesWithObsoleteMembers_;
    static NodeMultiMap cppClasses_;
    static NodeMultiMap qmlBasicTypes_;
    static NodeMultiMap qmlTypes_;
    static NodeMultiMap examples_;
    static NodeMapMap newClassMaps_;
    static NodeMapMap newQmlTypeMaps_;
    static NodeMultiMapMap newSinceMaps_;

    bool showInternal_;
    bool singleExec_;
    QString version_;
    QDocForest forest_;

    NodeMultiMap namespaceIndex_;
    NodeMultiMap attributions_;
    NodeMapMap functionIndex_;
    TextToNodeMap legaleseTexts_;
    QSet<QString> openNamespaces_;
};

QT_END_NAMESPACE

#endif
