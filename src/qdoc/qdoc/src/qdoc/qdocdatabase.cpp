// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdocdatabase.h"

#include "atom.h"
#include "collectionnode.h"
#include "functionnode.h"
#include "generator.h"
#include "genustypes.h"
#include "qdocindexfiles.h"
#include "tree.h"

#include <QtCore/qregularexpression.h>
#include <stack>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
static NodeMultiMap emptyNodeMultiMap_;

/*!
  \class QDocForest

  A class representing a forest of Tree objects.

  This private class manages a collection of Tree objects (a
  forest) for the singleton QDocDatabase object. It is only
  accessed by that singleton QDocDatabase object, which is a
  friend. Each tree in the forest is an instance of class
  Tree, which is a mostly private class. Both QDocForest and
  QDocDatabase are friends of Tree and have full access.

  There are two kinds of trees in the forest, differing not
  in structure but in use. One Tree is the primary tree. It
  is the tree representing the module being documented. All
  the other trees in the forest are called index trees. Each
  one represents the contents of the index file for one of
  the modules the current module must be able to link to.

  The instances of subclasses of Node in the primary tree
  will contain documentation in an instance of Doc. The
  index trees contain no documentation, and each Node in
  an index tree is marked as an index node.

  Each tree is named with the name of its module.

  The search order is created by searchOrder(), if it has
  not already been created. The search order and module
  names arrays have parallel structure, i.e. modulNames_[i]
  is the module name of the Tree at searchOrder_[i].

  The primary tree is always the first tree in the search
  order. i.e., when the database is searched, the primary
  tree is always searched first, unless a specific tree is
  being searched.
 */

/*!
  Destroys the qdoc forest. This requires deleting
  each Tree in the forest. Note that the forest has
  been transferred into the search order array, so
  what is really being used to destroy the forest
  is the search order array.
 */
QDocForest::~QDocForest()
{
    for (auto *entry : m_searchOrder)
        delete entry;
    m_forest.clear();
    m_searchOrder.clear();
    m_indexSearchOrder.clear();
    m_moduleNames.clear();
    m_primaryTree = nullptr;
}

/*!
  Initializes the forest prior to a traversal and
  returns a pointer to the primary tree. If the
  forest is empty, it returns \nullptr.
 */
Tree *QDocForest::firstTree()
{
    m_currentIndex = 0;
    return (!searchOrder().isEmpty() ? searchOrder()[0] : nullptr);
}

/*!
  Increments the forest's current tree index. If the current
  tree index is still within the forest, the function returns
  the pointer to the current tree. Otherwise it returns \nullptr.
 */
Tree *QDocForest::nextTree()
{
    ++m_currentIndex;
    return (m_currentIndex < searchOrder().size() ? searchOrder()[m_currentIndex] : nullptr);
}

/*!
  \fn Tree *QDocForest::primaryTree()

  Returns the pointer to the primary tree.
 */

/*!
  Finds the tree for module \a t in the forest and
  sets the primary tree to be that tree. After the
  primary tree is set, that tree is removed from the
  forest.

  \node It gets re-inserted into the forest after the
  search order is built.
 */
void QDocForest::setPrimaryTree(const QString &t)
{
    QString T = t.toLower();
    m_primaryTree = findTree(T);
    m_forest.remove(T);
    if (m_primaryTree == nullptr)
        qCCritical(lcQdoc) << "Error: Could not set primary tree to" << t;
}

/*!
  If the search order array is empty, create the search order.
  If the search order array is not empty, do nothing.
 */
void QDocForest::setSearchOrder(const QStringList &t)
{
    if (!m_searchOrder.isEmpty())
        return;

    /* Allocate space for the search order. */
    m_searchOrder.reserve(m_forest.size() + 1);
    m_searchOrder.clear();
    m_moduleNames.reserve(m_forest.size() + 1);
    m_moduleNames.clear();

    /* The primary tree is always first in the search order. */
    QString primaryName = primaryTree()->physicalModuleName();
    m_searchOrder.append(m_primaryTree);
    m_moduleNames.append(primaryName);
    m_forest.remove(primaryName);

    for (const QString &m : t) {
        if (primaryName != m) {
            auto it = m_forest.find(m);
            if (it != m_forest.end()) {
                m_searchOrder.append(it.value());
                m_moduleNames.append(m);
                m_forest.remove(m);
            }
        }
    }
    /*
      If any trees remain in the forest, just add them
      to the search order sequentially, because we don't
      know any better at this point.
     */
    if (!m_forest.isEmpty()) {
        for (auto it = m_forest.begin(); it != m_forest.end(); ++it) {
            m_searchOrder.append(it.value());
            m_moduleNames.append(it.key());
        }
        m_forest.clear();
    }

    /*
      Rebuild the forest after constructing the search order.
      It was destroyed during construction of the search order,
      but it is needed for module-specific searches.

      Note that this loop also inserts the primary tree into the
      forrest. That is a requirement.
     */
    for (int i = 0; i < m_searchOrder.size(); ++i) {
        if (!m_forest.contains(m_moduleNames.at(i))) {
            m_forest.insert(m_moduleNames.at(i), m_searchOrder.at(i));
        }
    }
}

/*!
  Returns an ordered array of Tree pointers that represents
  the order in which the trees should be searched. The first
  Tree in the array is the tree for the current module, i.e.
  the module for which qdoc is generating documentation.

  The other Tree pointers in the array represent the index
  files that were loaded in preparation for generating this
  module's documentation. Each Tree pointer represents one
  index file. The index file Tree points have been ordered
  heuristically to, hopefully, minimize searching. Thr order
  will probably be changed.

  If the search order array is empty, this function calls
  indexSearchOrder(). The search order array is empty while
  the index files are being loaded, but some searches must
  be performed during this time, notably searches for base
  class nodes. These searches require a temporary search
  order. The temporary order changes throughout the loading
  of the index files, but it is always the tree for the
  current index file first, followed by the trees for the
  index files that have already been loaded. The only
  ordering required in this temporary search order is that
  the current tree must be searched first.
 */
const QList<Tree *> &QDocForest::searchOrder()
{
    if (m_searchOrder.isEmpty())
        return indexSearchOrder();
    return m_searchOrder;
}

/*!
  There are two search orders used by qdoc when searching for
  things. The normal search order is returned by searchOrder(),
  but this normal search order is not known until all the index
  files have been read. At that point, setSearchOrder() is
  called.

  During the reading of the index files, the vector holding
  the normal search order remains empty. Whenever the search
  order is requested, if that vector is empty, this function
  is called to return a temporary search order, which includes
  all the index files that have been read so far, plus the
  one being read now. That one is prepended to the front of
  the vector.
 */
const QList<Tree *> &QDocForest::indexSearchOrder()
{
    if (m_forest.size() > m_indexSearchOrder.size())
        m_indexSearchOrder.prepend(m_primaryTree);
    return m_indexSearchOrder;
}

/*!
  Create a new Tree for the index file for the specified
  \a module and add it to the forest. Return the pointer
  to its root.
 */
NamespaceNode *QDocForest::newIndexTree(const QString &module)
{
    m_primaryTree = new Tree(module, m_qdb);
    m_forest.insert(module.toLower(), m_primaryTree);
    return m_primaryTree->root();
}

/*!
  Create a new Tree for use as the primary tree. This tree
  will represent the primary module. \a module is camel case.
 */
void QDocForest::newPrimaryTree(const QString &module)
{
    m_primaryTree = new Tree(module, m_qdb);
}

/*!
  Searches through the forest for a node named \a targetPath
  and returns a pointer to it if found. The \a relative node
  is the starting point. It only makes sense for the primary
  tree, which is searched first. After the primary tree has
  been searched, \a relative is set to 0 for searching the
  other trees, which are all index trees. With relative set
  to 0, the starting point for each index tree is the root
  of the index tree.

  If \a targetPath is resolved successfully but it refers to
  a \\section title, continue the search, keeping the section
  title as a fallback if no higher-priority targets are found.
 */
const Node *QDocForest::findNodeForTarget(QStringList &targetPath, const Node *relative,
                                          Genus genus, QString &ref)
{
    int flags = SearchBaseClasses | SearchEnumValues;

    QString entity = targetPath.takeFirst();
    QStringList entityPath = entity.split("::");

    QString target;
    if (!targetPath.isEmpty())
        target = targetPath.takeFirst();

    TargetRec::TargetType type = TargetRec::Unknown;
    const Node *tocNode = nullptr;
    for (const auto *tree : searchOrder()) {
        const Node *n = tree->findNodeForTarget(entityPath, target, relative, flags, genus, ref, &type);
        if (n) {
            // Targets referring to non-section titles are returned immediately
            if (type != TargetRec::Contents)
                return n;
            if (!tocNode)
                tocNode = n;
        }
        relative = nullptr;
    }
    return tocNode;
}

/*!
  Finds the FunctionNode for the qualified function name
  in \a path, that also has the specified \a parameters.
  Returns a pointer to the first matching function.

  \a relative is a node in the primary tree where the search
  should begin. It is only used when searching the primary
  tree. \a genus can be used to force the search to find a
  C++ function or a QML function.
 */
const FunctionNode *QDocForest::findFunctionNode(const QStringList &path,
                                                 const Parameters &parameters, const Node *relative,
                                                 Genus genus)
{
    for (const auto *tree : searchOrder()) {
        const FunctionNode *fn = tree->findFunctionNode(path, parameters, relative, genus);
        if (fn)
            return fn;
        relative = nullptr;
    }
    return nullptr;
}

/*! \class QDocDatabase
  This class provides exclusive access to the qdoc database,
  which consists of a forrest of trees and a lot of maps and
  other useful data structures.
 */

QDocDatabase *QDocDatabase::s_qdocDB = nullptr;
NodeMap QDocDatabase::s_typeNodeMap;
NodeMultiMap QDocDatabase::s_obsoleteClasses;
NodeMultiMap QDocDatabase::s_classesWithObsoleteMembers;
NodeMultiMap QDocDatabase::s_obsoleteQmlTypes;
NodeMultiMap QDocDatabase::s_qmlTypesWithObsoleteMembers;
NodeMultiMap QDocDatabase::s_cppClasses;
NodeMultiMap QDocDatabase::s_qmlBasicTypes;
NodeMultiMap QDocDatabase::s_qmlTypes;
NodeMultiMap QDocDatabase::s_examples;
NodeMultiMapMap QDocDatabase::s_newClassMaps;
NodeMultiMapMap QDocDatabase::s_newQmlTypeMaps;
NodeMultiMapMap QDocDatabase::s_newEnumValueMaps;
NodeMultiMapMap QDocDatabase::s_newSinceMaps;

/*!
  Constructs the singleton qdoc database object. The singleton
  constructs the \a forest_ object, which is also a singleton.
  \a m_showInternal is normally false. If it is true, qdoc will
  write documentation for nodes marked \c internal.

  \a singleExec_ is false when qdoc is being used in the standard
  way of running qdoc twices for each module, first with -prepare
  and then with -generate. First the -prepare phase is run for
  each module, then the -generate phase is run for each module.

  When \a singleExec_ is true, qdoc is run only once. During the
  single execution, qdoc processes the qdocconf files for all the
  modules sequentially in a loop. Each source file for each module
  is read exactly once.
 */
QDocDatabase::QDocDatabase() : m_forest(this)
{
    // nothing
}

/*!
  Creates the singleton. Allows only one instance of the class
  to be created. Returns a pointer to the singleton.
*/
QDocDatabase *QDocDatabase::qdocDB()
{
    if (s_qdocDB == nullptr) {
        s_qdocDB = new QDocDatabase;
        initializeDB();
    }
    return s_qdocDB;
}

/*!
  Destroys the singleton.
 */
void QDocDatabase::destroyQdocDB()
{
    if (s_qdocDB != nullptr) {
        delete s_qdocDB;
        s_qdocDB = nullptr;
    }
}

/*!
  Initialize data structures in the singleton qdoc database.

  In particular, the type node map is initialized with a lot
  type names that don't refer to documented types. For example,
  many C++ standard types are included. These might be documented
  here at some point, but for now they are not. Other examples
  include \c array and \c data, which are just generic names
  used as place holders in function signatures that appear in
  the documentation.

  \note Do not add QML basic types into this list as it will
        break linking to those types.
 */
void QDocDatabase::initializeDB()
{
    s_typeNodeMap.insert("accepted", nullptr);
    s_typeNodeMap.insert("actionPerformed", nullptr);
    s_typeNodeMap.insert("activated", nullptr);
    s_typeNodeMap.insert("alias", nullptr);
    s_typeNodeMap.insert("anchors", nullptr);
    s_typeNodeMap.insert("any", nullptr);
    s_typeNodeMap.insert("array", nullptr);
    s_typeNodeMap.insert("autoSearch", nullptr);
    s_typeNodeMap.insert("axis", nullptr);
    s_typeNodeMap.insert("backClicked", nullptr);
    s_typeNodeMap.insert("boomTime", nullptr);
    s_typeNodeMap.insert("border", nullptr);
    s_typeNodeMap.insert("buttonClicked", nullptr);
    s_typeNodeMap.insert("callback", nullptr);
    s_typeNodeMap.insert("char", nullptr);
    s_typeNodeMap.insert("clicked", nullptr);
    s_typeNodeMap.insert("close", nullptr);
    s_typeNodeMap.insert("closed", nullptr);
    s_typeNodeMap.insert("cond", nullptr);
    s_typeNodeMap.insert("data", nullptr);
    s_typeNodeMap.insert("dataReady", nullptr);
    s_typeNodeMap.insert("dateString", nullptr);
    s_typeNodeMap.insert("dateTimeString", nullptr);
    s_typeNodeMap.insert("datetime", nullptr);
    s_typeNodeMap.insert("day", nullptr);
    s_typeNodeMap.insert("deactivated", nullptr);
    s_typeNodeMap.insert("drag", nullptr);
    s_typeNodeMap.insert("easing", nullptr);
    s_typeNodeMap.insert("error", nullptr);
    s_typeNodeMap.insert("exposure", nullptr);
    s_typeNodeMap.insert("fatalError", nullptr);
    s_typeNodeMap.insert("fileSelected", nullptr);
    s_typeNodeMap.insert("flags", nullptr);
    s_typeNodeMap.insert("float", nullptr);
    s_typeNodeMap.insert("focus", nullptr);
    s_typeNodeMap.insert("focusZone", nullptr);
    s_typeNodeMap.insert("format", nullptr);
    s_typeNodeMap.insert("framePainted", nullptr);
    s_typeNodeMap.insert("from", nullptr);
    s_typeNodeMap.insert("frontClicked", nullptr);
    s_typeNodeMap.insert("function", nullptr);
    s_typeNodeMap.insert("hasOpened", nullptr);
    s_typeNodeMap.insert("hovered", nullptr);
    s_typeNodeMap.insert("hoveredTitle", nullptr);
    s_typeNodeMap.insert("hoveredUrl", nullptr);
    s_typeNodeMap.insert("imageCapture", nullptr);
    s_typeNodeMap.insert("imageProcessing", nullptr);
    s_typeNodeMap.insert("index", nullptr);
    s_typeNodeMap.insert("initialized", nullptr);
    s_typeNodeMap.insert("isLoaded", nullptr);
    s_typeNodeMap.insert("item", nullptr);
    s_typeNodeMap.insert("key", nullptr);
    s_typeNodeMap.insert("keysequence", nullptr);
    s_typeNodeMap.insert("listViewClicked", nullptr);
    s_typeNodeMap.insert("loadRequest", nullptr);
    s_typeNodeMap.insert("locale", nullptr);
    s_typeNodeMap.insert("location", nullptr);
    s_typeNodeMap.insert("long", nullptr);
    s_typeNodeMap.insert("message", nullptr);
    s_typeNodeMap.insert("messageReceived", nullptr);
    s_typeNodeMap.insert("mode", nullptr);
    s_typeNodeMap.insert("month", nullptr);
    s_typeNodeMap.insert("name", nullptr);
    s_typeNodeMap.insert("number", nullptr);
    s_typeNodeMap.insert("object", nullptr);
    s_typeNodeMap.insert("offset", nullptr);
    s_typeNodeMap.insert("ok", nullptr);
    s_typeNodeMap.insert("openCamera", nullptr);
    s_typeNodeMap.insert("openImage", nullptr);
    s_typeNodeMap.insert("openVideo", nullptr);
    s_typeNodeMap.insert("padding", nullptr);
    s_typeNodeMap.insert("parent", nullptr);
    s_typeNodeMap.insert("path", nullptr);
    s_typeNodeMap.insert("photoModeSelected", nullptr);
    s_typeNodeMap.insert("position", nullptr);
    s_typeNodeMap.insert("precision", nullptr);
    s_typeNodeMap.insert("presetClicked", nullptr);
    s_typeNodeMap.insert("preview", nullptr);
    s_typeNodeMap.insert("previewSelected", nullptr);
    s_typeNodeMap.insert("progress", nullptr);
    s_typeNodeMap.insert("puzzleLost", nullptr);
    s_typeNodeMap.insert("qmlSignal", nullptr);
    s_typeNodeMap.insert("rectangle", nullptr);
    s_typeNodeMap.insert("request", nullptr);
    s_typeNodeMap.insert("requestId", nullptr);
    s_typeNodeMap.insert("section", nullptr);
    s_typeNodeMap.insert("selected", nullptr);
    s_typeNodeMap.insert("send", nullptr);
    s_typeNodeMap.insert("settingsClicked", nullptr);
    s_typeNodeMap.insert("shoe", nullptr);
    s_typeNodeMap.insert("short", nullptr);
    s_typeNodeMap.insert("signed", nullptr);
    s_typeNodeMap.insert("sizeChanged", nullptr);
    s_typeNodeMap.insert("size_t", nullptr);
    s_typeNodeMap.insert("sockaddr", nullptr);
    s_typeNodeMap.insert("someOtherSignal", nullptr);
    s_typeNodeMap.insert("sourceSize", nullptr);
    s_typeNodeMap.insert("startButtonClicked", nullptr);
    s_typeNodeMap.insert("state", nullptr);
    s_typeNodeMap.insert("std::initializer_list", nullptr);
    s_typeNodeMap.insert("std::list", nullptr);
    s_typeNodeMap.insert("std::map", nullptr);
    s_typeNodeMap.insert("std::pair", nullptr);
    s_typeNodeMap.insert("std::string", nullptr);
    s_typeNodeMap.insert("std::vector", nullptr);
    s_typeNodeMap.insert("stringlist", nullptr);
    s_typeNodeMap.insert("swapPlayers", nullptr);
    s_typeNodeMap.insert("symbol", nullptr);
    s_typeNodeMap.insert("t", nullptr);
    s_typeNodeMap.insert("T", nullptr);
    s_typeNodeMap.insert("tagChanged", nullptr);
    s_typeNodeMap.insert("timeString", nullptr);
    s_typeNodeMap.insert("timeout", nullptr);
    s_typeNodeMap.insert("to", nullptr);
    s_typeNodeMap.insert("toggled", nullptr);
    s_typeNodeMap.insert("type", nullptr);
    s_typeNodeMap.insert("unsigned", nullptr);
    s_typeNodeMap.insert("urllist", nullptr);
    s_typeNodeMap.insert("va_list", nullptr);
    s_typeNodeMap.insert("value", nullptr);
    s_typeNodeMap.insert("valueEmitted", nullptr);
    s_typeNodeMap.insert("videoFramePainted", nullptr);
    s_typeNodeMap.insert("videoModeSelected", nullptr);
    s_typeNodeMap.insert("videoRecorder", nullptr);
    s_typeNodeMap.insert("void", nullptr);
    s_typeNodeMap.insert("volatile", nullptr);
    s_typeNodeMap.insert("wchar_t", nullptr);
    s_typeNodeMap.insert("x", nullptr);
    s_typeNodeMap.insert("y", nullptr);
    s_typeNodeMap.insert("zoom", nullptr);
    s_typeNodeMap.insert("zoomTo", nullptr);
}

/*! \fn NamespaceNode *QDocDatabase::primaryTreeRoot()
  Returns a pointer to the root node of the primary tree.
 */

/*!
  \fn const CNMap &QDocDatabase::groups()
  Returns a const reference to the collection of all
  group nodes in the primary tree.
*/

/*!
  \fn const CNMap &QDocDatabase::modules()
  Returns a const reference to the collection of all
  module nodes in the primary tree.
*/

/*!
  \fn const CNMap &QDocDatabase::qmlModules()
  Returns a const reference to the collection of all
  QML module nodes in the primary tree.
*/

/*! \fn CollectionNode *QDocDatabase::findGroup(const QString &name)
  Find the group node named \a name and return a pointer
  to it. If a matching node is not found, add a new group
  node named \a name and return a pointer to that one.

  If a new group node is added, its parent is the tree root,
  and the new group node is marked \e{not seen}.
 */

/*! \fn CollectionNode *QDocDatabase::findModule(const QString &name)
  Find the module node named \a name and return a pointer
  to it. If a matching node is not found, add a new module
  node named \a name and return a pointer to that one.

  If a new module node is added, its parent is the tree root,
  and the new module node is marked \e{not seen}.
 */

/*! \fn CollectionNode *QDocDatabase::addGroup(const QString &name)
  Looks up the group named \a name in the primary tree. If
  a match is found, a pointer to the node is returned.
  Otherwise, a new group node named \a name is created and
  inserted into the collection, and the pointer to that node
  is returned.
 */

/*! \fn CollectionNode *QDocDatabase::addModule(const QString &name)
  Looks up the module named \a name in the primary tree. If
  a match is found, a pointer to the node is returned.
  Otherwise, a new module node named \a name is created and
  inserted into the collection, and the pointer to that node
  is returned.
 */

/*! \fn CollectionNode *QDocDatabase::addQmlModule(const QString &name)
  Looks up the QML module named \a name in the primary tree.
  If a match is found, a pointer to the node is returned.
  Otherwise, a new QML module node named \a name is created
  and inserted into the collection, and the pointer to that
  node is returned.
 */

/*! \fn CollectionNode *QDocDatabase::addToGroup(const QString &name, Node *node)
  Looks up the group node named \a name in the collection
  of all group nodes. If a match is not found, a new group
  node named \a name is created and inserted into the collection.
  Then append \a node to the group's members list, and append the
  group node to the member list of the \a node. The parent of the
  \a node is not changed by this function. Returns a pointer to
  the group node.
 */

/*! \fn CollectionNode *QDocDatabase::addToModule(const QString &name, Node *node)
  Looks up the module node named \a name in the collection
  of all module nodes. If a match is not found, a new module
  node named \a name is created and inserted into the collection.
  Then append \a node to the module's members list. The parent of
  \a node is not changed by this function. Returns the module node.
 */

/*! \fn Collection *QDocDatabase::addToQmlModule(const QString &name, Node *node)
  Looks up the QML module named \a name. If it isn't there,
  create it. Then append \a node to the QML module's member
  list. The parent of \a node is not changed by this function.
 */

/*! \fn QmlTypeNode *QDocDatabase::findQmlType(const QString &name)
  Returns the QML type node identified by the qualified
  QML type \a name, or \c nullptr if no type was found.
 */

/*!
  Returns the QML type node identified by the QML module id
  \a qmid and QML type \a name, or \c nullptr if no type
  was found.

  If the QML module id is empty, looks up the QML type by
  \a name only.
 */
QmlTypeNode *QDocDatabase::findQmlType(const QString &qmid, const QString &name)
{
    if (!qmid.isEmpty()) {
        if (auto *qcn = m_forest.lookupQmlType(qmid + u"::"_s + name); qcn)
            return qcn;
    }

    QStringList path(name);
    return static_cast<QmlTypeNode *>(m_forest.findNodeByNameAndType(path, &Node::isQmlType));
}

/*!
  Returns the QML type node identified by the QML module id
  constructed from the strings in the import \a record and the
  QML type \a name. Returns \c nullptr if no type was not found.
 */
QmlTypeNode *QDocDatabase::findQmlType(const ImportRec &record, const QString &name)
{
    if (record.isEmpty())
        return nullptr;

    QString type{name};

    // If the import is under a namespace (id) and the type name is not prefixed with that id,
    // then we know the type is not available under this import.
    if (!record.m_importId.isEmpty()) {
        const QString namespacePrefix{"%1."_L1.arg(record.m_importId)};
        if (!type.startsWith(namespacePrefix))
            return nullptr;
        type.remove(0, namespacePrefix.size());
    }

    const QString qmName = record.m_importUri.isEmpty() ? record.m_moduleName : record.m_importUri;
    return m_forest.lookupQmlType(qmName + u"::"_s + type);
}

/*!
  Returns the QML node identified by the QML module id \a qmid
  and \a name, searching in the primary tree only. If \a qmid
  is an empty string, searches for the node using name only.

  Returns \c nullptr if no node was found.
*/
QmlTypeNode *QDocDatabase::findQmlTypeInPrimaryTree(const QString &qmid, const QString &name)
{
    if (!qmid.isEmpty())
        return primaryTree()->lookupQmlType(qmid + u"::"_s + name);
    return static_cast<QmlTypeNode *>(primaryTreeRoot()->findChildNode(name, Genus::QML, TypesOnly));
}

/*!
  This function calls a set of functions for each tree in the
  forest that has not already been analyzed. In this way, when
  running qdoc in \e singleExec mode, each tree is analyzed in
  turn, and its classes and types are added to the appropriate
  node maps.
 */
void QDocDatabase::processForest()
{
    processForest(&QDocDatabase::findAllClasses);
    processForest(&QDocDatabase::findAllFunctions);
    processForest(&QDocDatabase::findAllObsoleteThings);
    processForest(&QDocDatabase::findAllLegaleseTexts);
    processForest(&QDocDatabase::findAllSince);
    processForest(&QDocDatabase::findAllAttributions);
    resolveNamespaces();
}

/*!
  This function calls \a func for each tree in the forest,
  ensuring that \a func is called only once per tree.

  \sa processForest()
 */
void QDocDatabase::processForest(FindFunctionPtr func)
{
    Tree *t = m_forest.firstTree();
    while (t) {
        if (!m_completedFindFunctions.values(t).contains(func)) {
            (this->*(func))(t->root());
            m_completedFindFunctions.insert(t, func);
        }
        t = m_forest.nextTree();
    }
}

/*!
  Returns a reference to the collection of legalese texts.
 */
TextToNodeMap &QDocDatabase::getLegaleseTexts()
{
    processForest(&QDocDatabase::findAllLegaleseTexts);
    return m_legaleseTexts;
}

/*!
  Returns a reference to the map of C++ classes with obsolete members.
 */
NodeMultiMap &QDocDatabase::getClassesWithObsoleteMembers()
{
    processForest(&QDocDatabase::findAllObsoleteThings);
    return s_classesWithObsoleteMembers;
}

/*!
  Returns a reference to the map of obsolete QML types.
 */
NodeMultiMap &QDocDatabase::getObsoleteQmlTypes()
{
    processForest(&QDocDatabase::findAllObsoleteThings);
    return s_obsoleteQmlTypes;
}

/*!
  Returns a reference to the map of QML types with obsolete members.
 */
NodeMultiMap &QDocDatabase::getQmlTypesWithObsoleteMembers()
{
    processForest(&QDocDatabase::findAllObsoleteThings);
    return s_qmlTypesWithObsoleteMembers;
}

/*!
  Returns a reference to the map of QML basic types.
 */
NodeMultiMap &QDocDatabase::getQmlValueTypes()
{
    processForest(&QDocDatabase::findAllClasses);
    return s_qmlBasicTypes;
}

/*!
  Returns a reference to the multimap of QML types.
 */
NodeMultiMap &QDocDatabase::getQmlTypes()
{
    processForest(&QDocDatabase::findAllClasses);
    return s_qmlTypes;
}

/*!
  Returns a reference to the multimap of example nodes.
 */
NodeMultiMap &QDocDatabase::getExamples()
{
    processForest(&QDocDatabase::findAllClasses);
    return s_examples;
}

/*!
  Returns a reference to the multimap of attribution nodes.
 */
NodeMultiMap &QDocDatabase::getAttributions()
{
    processForest(&QDocDatabase::findAllAttributions);
    return m_attributions;
}

/*!
  Returns a reference to the map of obsolete C++ clases.
 */
NodeMultiMap &QDocDatabase::getObsoleteClasses()
{
    processForest(&QDocDatabase::findAllObsoleteThings);
    return s_obsoleteClasses;
}

/*!
  Returns a reference to the map of all C++ classes.
 */
NodeMultiMap &QDocDatabase::getCppClasses()
{
    processForest(&QDocDatabase::findAllClasses);
    return s_cppClasses;
}

/*!
  Returns the function index. This data structure is used to
  output the function index page.
 */
NodeMapMap &QDocDatabase::getFunctionIndex()
{
    processForest(&QDocDatabase::findAllFunctions);
    return m_functionIndex;
}

/*!
  Finds all the nodes containing legalese text and puts them
  in a map.
 */
void QDocDatabase::findAllLegaleseTexts(Aggregate *node)
{
    for (const auto &childNode : node->childNodes()) {
        if (childNode->isPrivate())
            continue;
        if (!childNode->doc().legaleseText().isEmpty())
            m_legaleseTexts.insert(childNode->doc().legaleseText(), childNode);
        if (childNode->isAggregate())
            findAllLegaleseTexts(static_cast<Aggregate *>(childNode));
    }
}

/*!
  \fn void QDocDatabase::findAllObsoleteThings(Aggregate *node)

  Finds all nodes with status = Deprecated and sorts them into
  maps. They can be C++ classes, QML types, or they can be
  functions, enum types, typedefs, methods, etc.
 */

/*!
  \fn void QDocDatabase::findAllSince(Aggregate *node)

  Finds all the nodes in \a node where a \e{since} command appeared
  in the qdoc comment and sorts them into maps according to the kind
  of node.

  This function is used for generating the "New Classes... in x.y"
  section on the \e{What's New in Qt x.y} page.
 */

/*!
  Find the \a key in the map of new class maps, and return a
  reference to the value, which is a NodeMap. If \a key is not
  found, return a reference to an empty NodeMap.
 */
const NodeMultiMap &QDocDatabase::getClassMap(const QString &key)
{
    processForest(&QDocDatabase::findAllSince);
    auto it = s_newClassMaps.constFind(key);
    return (it != s_newClassMaps.constEnd()) ? it.value() : emptyNodeMultiMap_;
}

/*!
  Find the \a key in the map of new QML type maps, and return a
  reference to the value, which is a NodeMap. If the \a key is not
  found, return a reference to an empty NodeMap.
 */
const NodeMultiMap &QDocDatabase::getQmlTypeMap(const QString &key)
{
    processForest(&QDocDatabase::findAllSince);
    auto it = s_newQmlTypeMaps.constFind(key);
    return (it != s_newQmlTypeMaps.constEnd()) ? it.value() : emptyNodeMultiMap_;
}

/*!
  Find the \a key in the map of new \e {since} maps, and return
  a reference to the value, which is a NodeMultiMap. If \a key
  is not found, return a reference to an empty NodeMultiMap.
 */
const NodeMultiMap &QDocDatabase::getSinceMap(const QString &key)
{
    processForest(&QDocDatabase::findAllSince);
    auto it = s_newSinceMaps.constFind(key);
    return (it != s_newSinceMaps.constEnd()) ? it.value() : emptyNodeMultiMap_;
}

/*!
  Performs several housekeeping tasks prior to generating the
  documentation. These tasks create required data structures
  and resolve links.
 */
void QDocDatabase::resolveStuff()
{
    const auto &config = Config::instance();
    if (config.dualExec() || config.preparing()) {
        // order matters
        primaryTree()->resolveBaseClasses(primaryTreeRoot());
        primaryTree()->resolvePropertyOverriddenFromPtrs(primaryTreeRoot());
        primaryTreeRoot()->resolveRelates();
        primaryTreeRoot()->normalizeOverloads();
        primaryTree()->markDontDocumentNodes();
        primaryTree()->removePrivateAndInternalBases(primaryTreeRoot());
        primaryTree()->resolveProperties();
        primaryTreeRoot()->markUndocumentedChildrenInternal();
        primaryTreeRoot()->resolveQmlInheritance();
        primaryTree()->resolveTargets(primaryTreeRoot());
        primaryTree()->resolveCppToQmlLinks();
        primaryTree()->resolveSince(*primaryTreeRoot());
    }
    if (config.singleExec() && config.generating()) {
        primaryTree()->resolveBaseClasses(primaryTreeRoot());
        primaryTree()->resolvePropertyOverriddenFromPtrs(primaryTreeRoot());
        primaryTreeRoot()->resolveQmlInheritance();
        primaryTree()->resolveCppToQmlLinks();
        primaryTree()->resolveSince(*primaryTreeRoot());
    }
    if (!config.preparing()) {
        resolveNamespaces();
        resolveProxies();
        resolveBaseClasses();
        updateNavigation();
    }
    if (config.dualExec())
        QDocIndexFiles::destroyQDocIndexFiles();
}

void QDocDatabase::resolveBaseClasses()
{
    Tree *t = m_forest.firstTree();
    while (t) {
        t->resolveBaseClasses(t->root());
        t = m_forest.nextTree();
    }
}

/*!
  Returns a reference to the namespace map. Constructs the
  namespace map if it hasn't been constructed yet.

  \note This function must not be called in the prepare phase.
 */
NodeMultiMap &QDocDatabase::getNamespaces()
{
    resolveNamespaces();
    return m_namespaceIndex;
}

/*!
  Multiple namespace nodes for namespace X can exist in the
  qdoc database in different trees. This function first finds
  all namespace nodes in all the trees and inserts them into
  a multimap. Then it combines all the namespace nodes that
  have the same name into a single namespace node of that
  name and inserts that combined namespace node into an index.
 */
void QDocDatabase::resolveNamespaces()
{
    if (!m_namespaceIndex.isEmpty())
        return;

    bool linkErrors = !Config::instance().get(CONFIG_NOLINKERRORS).asBool();
    NodeMultiMap namespaceMultimap;
    Tree *t = m_forest.firstTree();
    while (t) {
        t->root()->findAllNamespaces(namespaceMultimap);
        t = m_forest.nextTree();
    }
    const QList<QString> keys = namespaceMultimap.uniqueKeys();
    for (const QString &key : keys) {
        NamespaceNode *ns = nullptr;
        NamespaceNode *indexNamespace = nullptr;
        const NodeList namespaces = namespaceMultimap.values(key);
        qsizetype count = namespaceMultimap.remove(key);
        if (count > 0) {
            for (auto *node : namespaces) {
                ns = static_cast<NamespaceNode *>(node);
                if (ns->isDocumentedHere())
                    break;
                else if (ns->hadDoc())
                    indexNamespace = ns; // namespace was documented but in another tree
                ns = nullptr;
            }
            if (ns) {
                for (auto *node : namespaces) {
                    auto *nsNode = static_cast<NamespaceNode *>(node);
                    if (nsNode->hadDoc() && nsNode != ns) {
                        ns->doc().location().warning(
                                QStringLiteral("Namespace %1 documented more than once")
                                        .arg(nsNode->name()), QStringLiteral("also seen here: %1")
                                                .arg(nsNode->doc().location().toString()));
                    }
                }
            } else if (!indexNamespace) {
                // Warn about documented children in undocumented namespaces.
                // As the namespace can be documented outside this project,
                // skip the warning if -no-link-errors is set
                if (linkErrors) {
                    for (auto *node : namespaces) {
                        if (!node->isIndexNode())
                            static_cast<NamespaceNode *>(node)->reportDocumentedChildrenInUndocumentedNamespace();
                    }
                }
            } else {
                for (auto *node : namespaces) {
                    auto *nsNode = static_cast<NamespaceNode *>(node);
                    if (nsNode != indexNamespace)
                        nsNode->setDocNode(indexNamespace);
                }
            }
        }
        /*
          If there are multiple namespace nodes with the same
          name where one of them will be the main reference page
          for the namespace, include all nodes in the public
          API of the namespace.
         */
        if (ns && count > 1) {
            for (auto *node : namespaces) {
                auto *nameSpaceNode = static_cast<NamespaceNode *>(node);
                if (nameSpaceNode != ns) {
                    for (auto it = nameSpaceNode->constBegin(); it != nameSpaceNode->constEnd();
                         ++it) {
                        Node *anotherNs = *it;
                        if (anotherNs && anotherNs->isPublic() && !anotherNs->isInternal())
                            ns->includeChild(anotherNs);
                    }
                }
            }
        }
        /*
           Add the main namespace reference node to index, or the last seen
           namespace if the main one was not found.
        */
        if (!ns)
            ns = indexNamespace ? indexNamespace : static_cast<NamespaceNode *>(namespaces.last());
        m_namespaceIndex.insert(ns->name(), ns);
    }
}

/*!
  Each instance of class Tree that represents an index file
  must be traversed to find all instances of class ProxyNode.
  For each ProxyNode found, look up the ProxyNode's name in
  the primary Tree. If it is found, it means that the proxy
  node contains elements (normally just functions) that are
  documented in the module represented by the Tree containing
  the proxy node but that are related to the node we found in
  the primary tree.
 */
void QDocDatabase::resolveProxies()
{
    // The first tree is the primary tree.
    // Skip the primary tree.
    Tree *t = m_forest.firstTree();
    t = m_forest.nextTree();
    while (t) {
        const NodeList &proxies = t->proxies();
        if (!proxies.isEmpty()) {
            for (auto *node : proxies) {
                const auto *pn = static_cast<ProxyNode *>(node);
                if (pn->count() > 0) {
                    Aggregate *aggregate = primaryTree()->findAggregate(pn->name());
                    if (aggregate != nullptr)
                        aggregate->appendToRelatedByProxy(pn->childNodes());
                }
            }
        }
        t = m_forest.nextTree();
    }
}

/*!
  Finds the function node for the qualified function path in
  \a target and returns a pointer to it. The \a target is a
  function signature with or without parameters but without
  the return type.

  \a relative is the node in the primary tree where the search
  begins. It is not used in the other trees, if the node is not
  found in the primary tree. \a genus can be used to force the
  search to find a C++ function or a QML function.

  The entire forest is searched, but the first match is accepted.
 */
const FunctionNode *QDocDatabase::findFunctionNode(const QString &target, const Node *relative,
                                                   Genus genus)
{
    QString signature;
    QString function = target;
    qsizetype length = target.size();
    if (function.endsWith("()"))
        function.chop(2);
    if (function.endsWith(QChar(')'))) {
        qsizetype position = function.lastIndexOf(QChar('('));
        signature = function.mid(position + 1, length - position - 2);
        function = function.left(position);
    }
    QStringList path = function.split("::");
    return m_forest.findFunctionNode(path, Parameters(signature), relative, genus);
}

/*!
  This function is called for autolinking to a \a type,
  which could be a function return type or a parameter
  type. The tree node that represents the \a type is
  returned. All the trees are searched until a match is
  found. When searching the primary tree, the search
  begins at \a relative and proceeds up the parent chain.
  When searching the index trees, the search begins at the
  root.
 */
const Node *QDocDatabase::findTypeNode(const QString &type, const Node *relative, Genus genus)
{
    QStringList path = type.split("::");
    if ((path.size() == 1) && (path.at(0)[0].isLower() || path.at(0) == QString("T"))) {
        auto it = s_typeNodeMap.find(path.at(0));
        if (it != s_typeNodeMap.end())
            return it.value();
    }
    return m_forest.findTypeNode(path, relative, genus);
}

/*!
  Finds the node that will generate the documentation that
  contains the \a target and returns a pointer to it.

  Can this be improved by using the target map in Tree?
 */
const Node *QDocDatabase::findNodeForTarget(const QString &target, const Node *relative)
{
    const Node *node = nullptr;
    if (target.isEmpty())
        node = relative;
    else if (target.endsWith(".html"))
        node = findNodeByNameAndType(QStringList(target), &Node::isPageNode);
    else {
        QStringList path = target.split("::");
        int flags = SearchBaseClasses | SearchEnumValues;
        for (const auto *tree : searchOrder()) {
            const Node *n = tree->findNode(path, relative, flags, Genus::DontCare);
            if (n)
                return n;
            relative = nullptr;
        }
        node = findPageNodeByTitle(target);
    }
    return node;
}

QStringList QDocDatabase::groupNamesForNode(Node *node)
{
    QStringList result;
    CNMap *m = primaryTree()->getCollectionMap(NodeType::Group);

    if (!m)
        return result;

    for (auto it = m->cbegin(); it != m->cend(); ++it)
        if (it.value()->members().contains(node))
            result << it.key();

    return result;
}

/*!
  Reads and parses the qdoc index files listed in \a indexFiles.
 */
void QDocDatabase::readIndexes(const QStringList &indexFiles)
{
    QStringList filesToRead;
    for (const QString &file : indexFiles) {
        QString fn = file.mid(file.lastIndexOf(QChar('/')) + 1);
        if (!isLoaded(fn))
            filesToRead << file;
        else
            qCCritical(lcQdoc) << "Index file" << file << "is already in memory.";
    }
    QDocIndexFiles::qdocIndexFiles()->readIndexes(filesToRead);
}

/*!
  Generates a qdoc index file and write it to \a fileName. The
  index file is generated with the parameters \a url and \a title,
  using the generator \a g.
 */
void QDocDatabase::generateIndex(const QString &fileName, const QString &url, const QString &title)
{
    QString t = fileName.mid(fileName.lastIndexOf(QChar('/')) + 1);
    primaryTree()->setIndexFileName(t);
    QDocIndexFiles::qdocIndexFiles()->generateIndex(fileName, url, title);
    QDocIndexFiles::destroyQDocIndexFiles();
}

/*!
    Returns the collection node representing the module that \a relative
    node belongs to, or \c nullptr if there is no such module in the
    primary tree.
*/
const CollectionNode *QDocDatabase::getModuleNode(const Node *relative)
{
    NodeType moduleType{NodeType::Module};
    QString moduleName;
    switch (relative->genus())
    {
    case Genus::CPP:
        moduleType = NodeType::Module;
        moduleName = relative->physicalModuleName();
        break;
    case Genus::QML:
        moduleType = NodeType::QmlModule;
        moduleName = relative->logicalModuleName();
        break;
    default:
        return nullptr;
    }
    if (moduleName.isEmpty())
        return nullptr;

    return primaryTree()->getCollection(moduleName, moduleType);
}

/*!
  Finds all the collection nodes of the specified \a type
  and merges them into the collection node map \a cnm. Nodes
  that match the \a relative node are not included.
 */
void QDocDatabase::mergeCollections(NodeType type, CNMap &cnm, const Node *relative)
{
    cnm.clear();
    CNMultiMap cnmm;
    for (auto *tree : searchOrder()) {
        CNMap *m = tree->getCollectionMap(type);
        if (m && !m->isEmpty()) {
            for (auto it = m->cbegin(); it != m->cend(); ++it) {
                if (!it.value()->isInternal())
                    cnmm.insert(it.key(), it.value());
            }
        }
    }
    if (cnmm.isEmpty())
        return;
    static const QRegularExpression singleDigit("\\b([0-9])\\b");
    const QStringList keys = cnmm.uniqueKeys();
    for (const auto &key : keys) {
        const QList<CollectionNode *> values = cnmm.values(key);
        CollectionNode *n = nullptr;
        for (auto *value : values) {
            if (value && value->wasSeen() && value != relative) {
                n = value;
                break;
            }
        }
        if (n) {
            if (values.size() > 1) {
                for (CollectionNode *value : values) {
                    if (value != n) {
                        // Allow multiple (major) versions of QML modules
                        if ((n->isQmlModule())
                            && n->logicalModuleIdentifier() != value->logicalModuleIdentifier()) {
                            if (value->wasSeen() && value != relative)
                                cnm.insert(value->fullTitle().toLower(), value);
                            continue;
                        }
                        for (Node *t : value->members())
                            n->addMember(t);
                    }
                }
            }
            QString sortKey = n->fullTitle().toLower();
            if (sortKey.startsWith("the "))
                sortKey.remove(0, 4);
            sortKey.replace(singleDigit, "0\\1");
            cnm.insert(sortKey, n);
        }
    }
}

/*!
  Finds all the collection nodes with the same name
  and type as \a c and merges their members into the
  members list of \a c.

  For QML modules, only nodes with matching
  module identifiers are merged to avoid merging
  modules with different (major) versions.
 */
void QDocDatabase::mergeCollections(CollectionNode *c)
{
    if (c == nullptr)
        return;

    // REMARK: This form of merging is usually called during the
    // generation phase om-the-fly when a source-of-truth collection
    // is required.
    // In practice, this means a collection could be merged many, many
    // times during the lifetime of a generation.
    // To avoid repeating the merging process each time, which could
    // be time consuming, we use a small flag that is set directly on
    // the collection to bail-out early.
    //
    // The merging process is only meaningful for collections when the
    // collection references are spread troughout multiple projects.
    // The part of information that exists in other project is read
    // before the generation phase, such that when the generation
    // phase comes, we already have all the information we need for
    // merging such that we can consider all version of a certain
    // collection node immutable, making the caching inherently
    // correct at any point of the generation.
    //
    // This implies that this operation is unsafe if it is performed
    // before all the index files are loaded.
    // Indeed, this is a prerequisite, with the current structure, to
    // perform this optmization.
    //
    // At the current time, this is true and is expected not to
    // change.
    //
    // Do note that this is not applied to the other overload of
    // mergeCollections as we cannot as safely ensure its consistency
    // and, as the result of the merging depends on multiple
    // parameters, it would require an actual memoization of the call.
    //
    // Note that this is a defensive optimization and we are assuming
    // that it is effective based on heuristical data. As this is
    // expected to disappear, at least in its current form, in the
    // future, a more thorough analysis was not performed.
    if (c->isMerged()) {
        return;
    }

    for (auto *tree : searchOrder()) {
        CollectionNode *cn = tree->getCollection(c->name(), c->nodeType());
        if (cn && cn != c) {
            if ((cn->isQmlModule())
                && cn->logicalModuleIdentifier() != c->logicalModuleIdentifier())
                continue;

            for (auto *node : cn->members())
                c->addMember(node);

            // REMARK: The merging process is performed to ensure that
            // references to the collection in external projects are
            // taken into account before consuming the collection.
            //
            // This works by having QDoc construct empty collections
            // as soon as a reference to a collection is encountered
            // and filling details later on when its definition is
            // found.
            //
            // This initially-empty collection is always saved to the
            // primaryTree and it is the collection that is directly
            // accessible to consumers during the generation process.
            //
            // Nonetheless, when the definition for the collection is
            // not in the same project as the one that is being
            // compiled, its details will never be filled in.
            //
            // Indeed, the details will live in the index file for the
            // project where the collection is defined, if any, and
            // the node for it, which has complete information, will
            // live in some non-primaryTree.
            //
            // The merging process itself is used by consumers during
            // the generation process because they access the
            // primaryTree version of the collection expecting a
            // source-of-truth.
            // To ensure that this is the case for usages that
            // requires linking, we need to merge not only the members
            // of the collection that reside in external versions of
            // the collection; but some of the data that reside in the
            // definition of the collection intself, namely the title
            // and the url.
            //
            // A collection that contains the data of a definition is
            // always marked as seen, hence we use that to discern
            // whether we are working with a placeholder node or not,
            // and fill in the data if we encounter a node that
            // represents a definition.
            //
            // The way in which QDoc works implies that collection are
            // globally scoped between projects.
            // The repetition of the definition for the same
            // collection is warned for as a duplicate documentation,
            // such that we can expect a single valid source of truth
            // for a given collection in each project.
            // It is currently unknown if this warning is applicable
            // when the repeated collection is defined in two
            // different projects.
            //
            // As QDoc implicitly would not correctly support this
            // case, we assume that only one declaration exists for
            // each collection, such that the first encoutered one
            // must be the source of truth and that there is no need
            // to copy any data after the first copy is performed.
            // KLUDGE: Note that this process is done as a hackish
            // solution to QTBUG-104237 and should not be considered
            // final or dependable.
            if (!c->wasSeen() && cn->wasSeen()) {
                c->markSeen();
                c->setTitle(cn->title());
                c->setUrl(cn->url());
            }
        }
    }

    c->markMerged();
}

/*!
  Searches for the node that matches the path in \a atom and the
  specified \a genus. The \a relative node is used if the first
  leg of the path is empty, i.e. if the path begins with '#'.
  The function also sets \a ref if there remains an unused leg
  in the path after the node is found. The node is returned as
  well as the \a ref. If the returned node pointer is null,
  \a ref is also not valid.
 */
const Node *QDocDatabase::findNodeForAtom(const Atom *a, const Node *relative, QString &ref,
                                          Genus genus)
{
    const Node *node = nullptr;

    Atom *atom = const_cast<Atom *>(a);
    QStringList targetPath = atom->string().split(QLatin1Char('#'));
    QString first = targetPath.first().trimmed();

    Tree *domain = nullptr;

    if (atom->isLinkAtom()) {
        domain = atom->domain();
        genus = atom->genus();
    }

    if (first.isEmpty())
        node = relative; // search for a target on the current page.
    else if (domain) {
        if (first.endsWith(".html"))
            node = domain->findNodeByNameAndType(QStringList(first), &Node::isPageNode);
        else if (first.endsWith(QChar(')'))) {
            QString signature;
            QString function = first;
            qsizetype length = first.size();
            if (function.endsWith("()"))
                function.chop(2);
            if (function.endsWith(QChar(')'))) {
                qsizetype position = function.lastIndexOf(QChar('('));
                signature = function.mid(position + 1, length - position - 2);
                function = function.left(position);
            }
            QStringList path = function.split("::");
            node = domain->findFunctionNode(path, Parameters(signature), nullptr, genus);
        }
        if (node == nullptr) {
            int flags = SearchBaseClasses | SearchEnumValues;
            QStringList nodePath = first.split("::");
            QString target;
            targetPath.removeFirst();
            if (!targetPath.isEmpty())
                target = targetPath.takeFirst();
            if (relative && relative->tree()->physicalModuleName() != domain->physicalModuleName())
                relative = nullptr;
            return domain->findNodeForTarget(nodePath, target, relative, flags, genus, ref);
        }
    } else {
        if (first.endsWith(".html"))
            node = findNodeByNameAndType(QStringList(first), &Node::isPageNode);
        else if (first.endsWith(QChar(')')))
            node = findFunctionNode(first, relative, genus);
        if (node == nullptr)
            return findNodeForTarget(targetPath, relative, genus, ref);
    }

    if (node != nullptr && ref.isEmpty()) {
        if (!node->url().isEmpty())
            return node;
        targetPath.removeFirst();
        if (!targetPath.isEmpty()) {
            ref = node->root()->tree()->getRef(targetPath.first(), node);
            if (ref.isEmpty())
                node = nullptr;
        }
    }
    return node;
}

/*!
    Updates navigation (previous/next page links and the navigation parent)
    for pages listed in the TOC, specified by the \c navigation.toctitles
    configuration variable.

    if \c navigation.toctitles.inclusive is \c true, include also the TOC
    page(s) themselves as a 'root' item in the navigation bar (breadcrumbs)
    that are generated for HTML output.
*/
void QDocDatabase::updateNavigation()
{
    // Restrict searching only to the local (primary) tree
    QList<Tree *> searchOrder = this->searchOrder();
    setLocalSearch();

    const QString configVar = CONFIG_NAVIGATION +
                              Config::dot +
                              CONFIG_TOCTITLES;

    // TODO: [direct-configuration-access]
    // The configuration is currently a singleton with some generally
    // global mutable state.
    //
    // Accessing the data in this form complicates testing and
    // requires tests that inhibit any test parallelization, as the
    // tests are not self contained.
    //
    // This should be generally avoived. Possibly, we should strive
    // for Config to be a POD type that generally is scoped to
    // main and whose data is destructured into dependencies when
    // the dependencies are constructed.
    bool inclusive{Config::instance().get(
            configVar + Config::dot + CONFIG_INCLUSIVE).asBool()};

    // TODO: [direct-configuration-access]
    const auto tocTitles{Config::instance().get(configVar).asStringList()};

    for (const auto &tocTitle : tocTitles) {
        if (const auto candidateTarget = findNodeForTarget(tocTitle, nullptr); candidateTarget && candidateTarget->isPageNode()) {
            auto tocPage{static_cast<const PageNode*>(candidateTarget)};

            Text body = tocPage->doc().body();

            auto *atom = body.firstAtom();

            std::pair<PageNode *, QString> prev { nullptr, QString() };

            std::stack<const PageNode *> tocStack;
            tocStack.push(inclusive ? tocPage : nullptr);

            bool inItem = false;

            // TODO: Understand how much we use this form of looping over atoms.
            // If it is used a few times we might consider providing
            // an iterator for Text to make use of a simpler
            // range-for loop.
            while (atom) {
                switch (atom->type()) {
                    case Atom::ListItemLeft:
                        // Not known if we're going to have a link, push a temporary
                        tocStack.push(nullptr);
                        inItem = true;
                        break;
                    case Atom::ListItemRight:
                        tocStack.pop();
                        inItem = false;
                        break;
                    case Atom::Link: {
                        if (!inItem)
                            break;

                        // TODO: [unnecessary-output-parameter]
                        // We currently need an lvalue string to
                        // pass to findNodeForAtom, as the
                        // outparameter ref.
                        //
                        // Apart from the general problems with output
                        // parameters, we shouldn't be forced to
                        // instanciate an unnecessary object at call
                        // site.
                        //
                        // Understand what the correct way to avoid this is.
                        // This requires changes to findNodeForAtom
                        // and should be addressed in the context of
                        // revising that method.
                        QString unused{};
                        // TODO: Having to const cast is really a code
                        // smell and could result in undefined
                        // behavior in some specific cases (e.g point
                        // to something that is actually const).
                        //
                        // We should understand how to sequence the
                        // code so that we have access to mutable data
                        // when we need it and "freeze" the data
                        // afterwards.
                        //
                        // If it we expect this form of mutability at
                        // this point we should expose a non-const API
                        // for the database, possibly limited to a
                        // very specific scope of execution.
                        //
                        // Understand what the correct sequencing for
                        // this processing is and revise this part.
                        auto candidatePage = const_cast<Node *>(findNodeForAtom(atom, nullptr, unused));
                        if (!candidatePage || !candidatePage->isPageNode()) break;

                        auto page{static_cast<PageNode*>(candidatePage)};

                        // ignore self-references
                        if (page == prev.first) break;

                        if (prev.first) {
                            prev.first->setLink(
                                Node::NextLink,
                                page->title(),
                                // TODO: [possible-assertion-failure][imprecise-types][atoms-link]
                                // As with other structures in QDoc we
                                // are able to call methods that are
                                // valid only on very specific states.
                                //
                                // For some of those calls we have
                                // some defensive programming measures
                                // that allow us to at least identify
                                // the error during debugging, while
                                // for others this may currently hide
                                // some logic error.
                                //
                                // To avoid those cases, we should
                                // strive to move those cases to a
                                // compilation error, requiring a
                                // statically analyzable state that
                                // represents the current model.
                                //
                                // This would ensure that those
                                // lingering class of bugs are
                                // eliminated completely, forces a
                                // more explicit codebase where the
                                // current capabilities do not depend
                                // on runtime values might not be
                                // generally visible, and does not
                                // require us to incur into the
                                // required state, which may be rare,
                                // simplifying our abilities to
                                // evaluate all possible states.
                                //
                                // For linking atoms, LinkAtom is
                                // available and might be a good
                                // enough solution to move linkText
                                // to.
                                atom->linkText()
                            );
                            page->setLink(
                                Node::PreviousLink,
                                prev.first->title(),
                                prev.second
                            );
                        }

                        if (page == tocPage)
                            break;

                        // Find the navigation parent from the stack; we may have null pointers
                        // for non-link list items, so skip those.
                        qsizetype popped = 0;
                        while (tocStack.size() > 1 && !tocStack.top()) {
                            tocStack.pop();
                            ++popped;
                        }

                        page->setNavigationParent(tocStack.empty() ? nullptr : tocStack.top());

                        while (--popped > 0)
                            tocStack.push(nullptr);

                        tocStack.push(page);
                        // TODO: [possible-assertion-failure][imprecise-types][atoms-link]
                        prev = { page, atom->linkText() };
                    }
                    break;

                    case Atom::AnnotatedList:
                    case Atom::GeneratedList: {
                        if (const auto *cn = getCollectionNode(atom->string(), NodeType::Group)) {
                            const auto sortOrder{Generator::sortOrder(atom->strings().last())};
                            NodeList members{cn->members()};
                            // Drop non-page nodes and index nodes so that we do not generate navigational
                            // links pointing outside of this documentation set.
                            members.erase(std::remove_if(members.begin(), members.end(),
                                    [](const Node *n) {
                                        return n->isIndexNode() || !n->isPageNode() || n->isExternalPage();
                                    }), members.end());
                            if (members.isEmpty())
                                break;

                            if (sortOrder == Qt::DescendingOrder)
                                std::sort(members.rbegin(), members.rend(), Node::nodeSortKeyOrNameLessThan);
                            else
                                std::sort(members.begin(), members.end(), Node::nodeSortKeyOrNameLessThan);

                            // `members` now has local PageNode pointers, adjust prev/next links for each.
                            // Do not set a navigation parent node as group members use the group node as
                            // their nav. parent.
                            for (auto *m : members) {
                                auto *page = static_cast<PageNode *>(m);
                                prev.first->setLink(Node::NextLink, page->title(), page->fullName());
                                page->setLink(Node::PreviousLink, prev.first->title(), prev.second);
                                prev = { page, page->fullName() };
                            }
                        }
                    }
                    break;

                    default:
                        break;
                }

                atom = atom->next();
            }
        } else {
            Config::instance().get(configVar).location()
                    .warning(QStringLiteral("Failed to find table of contents with title '%1'")
                            .arg(tocTitle));
        }
    }

    // Restore search order
    setSearchOrder(searchOrder);
}

QT_END_NAMESPACE
