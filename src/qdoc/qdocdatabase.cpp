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

#include "qdocdatabase.h"

#include "atom.h"
#include "generator.h"
#include "qdocindexfiles.h"
#include "qdoctagfiles.h"
#include "tree.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

static NodeMap emptyNodeMap_;
static NodeMultiMap emptyNodeMultiMap_;
bool QDocDatabase::debug = false;

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
    for (int i = 0; i < searchOrder_.size(); ++i)
        delete searchOrder_.at(i);
    forest_.clear();
    searchOrder_.clear();
    indexSearchOrder_.clear();
    moduleNames_.clear();
    primaryTree_ = nullptr;
}

/*!
  Initializes the forest prior to a traversal and
  returns a pointer to the root node of the primary
  tree. If the forest is empty, it return 0
 */
NamespaceNode *QDocForest::firstRoot()
{
    currentIndex_ = 0;
    return (!searchOrder().isEmpty() ? searchOrder()[0]->root() : nullptr);
}

/*!
  Increments the forest's current tree index. If the current
  tree index is still within the forest, the function returns
  the root node of the current tree. Otherwise it returns 0.
 */
NamespaceNode *QDocForest::nextRoot()
{
    ++currentIndex_;
    return (currentIndex_ < searchOrder().size() ? searchOrder()[currentIndex_]->root() : nullptr);
}

/*!
  Initializes the forest prior to a traversal and
  returns a pointer to the primary tree. If the
  forest is empty, it returns 0.
 */
Tree *QDocForest::firstTree()
{
    currentIndex_ = 0;
    return (!searchOrder().isEmpty() ? searchOrder()[0] : nullptr);
}

/*!
  Increments the forest's current tree index. If the current
  tree index is still within the forest, the function returns
  the pointer to the current tree. Otherwise it returns 0.
 */
Tree *QDocForest::nextTree()
{
    ++currentIndex_;
    return (currentIndex_ < searchOrder().size() ? searchOrder()[currentIndex_] : nullptr);
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
    primaryTree_ = findTree(T);
    forest_.remove(T);
    if (primaryTree_ == nullptr)
        qDebug() << "ERROR: Could not set primary tree to:" << t;
}

/*!
  If the search order array is empty, create the search order.
  If the search order array is not empty, do nothing.
 */
void QDocForest::setSearchOrder(const QStringList &t)
{
    if (!searchOrder_.isEmpty())
        return;

    /* Allocate space for the search order. */
    searchOrder_.reserve(forest_.size() + 1);
    searchOrder_.clear();
    moduleNames_.reserve(forest_.size() + 1);
    moduleNames_.clear();

    /* The primary tree is always first in the search order. */
    QString primaryName = primaryTree()->physicalModuleName();
    searchOrder_.append(primaryTree_);
    moduleNames_.append(primaryName);
    forest_.remove(primaryName);

    for (const QString &m : t) {
        if (primaryName != m) {
            auto it = forest_.find(m);
            if (it != forest_.end()) {
                searchOrder_.append(it.value());
                moduleNames_.append(m);
                forest_.remove(m);
            }
        }
    }
    /*
      If any trees remain in the forest, just add them
      to the search order sequentially, because we don't
      know any better at this point.
     */
    if (!forest_.isEmpty()) {
        for (auto it = forest_.begin(); it != forest_.end(); ++it) {
            searchOrder_.append(it.value());
            moduleNames_.append(it.key());
        }
        forest_.clear();
    }

    /*
      Rebuild the forest after constructing the search order.
      It was destroyed during construction of the search order,
      but it is needed for module-specific searches.

      Note that this loop also inserts the primary tree into the
      forrest. That is a requirement.
     */
    for (int i = 0; i < searchOrder_.size(); ++i) {
        if (!forest_.contains(moduleNames_.at(i))) {
            forest_.insert(moduleNames_.at(i), searchOrder_.at(i));
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
const QVector<Tree *> &QDocForest::searchOrder()
{
    if (searchOrder_.isEmpty())
        return indexSearchOrder();
    return searchOrder_;
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
const QVector<Tree *> &QDocForest::indexSearchOrder()
{
    if (forest_.size() > indexSearchOrder_.size())
        indexSearchOrder_.prepend(primaryTree_);
    return indexSearchOrder_;
}

/*!
  Create a new Tree for the index file for the specified
  \a module and add it to the forest. Return the pointer
  to its root.
 */
NamespaceNode *QDocForest::newIndexTree(const QString &module)
{
    primaryTree_ = new Tree(module, qdb_);
    forest_.insert(module.toLower(), primaryTree_);
    return primaryTree_->root();
}

/*!
  Create a new Tree for use as the primary tree. This tree
  will represent the primary module. \a module is camel case.
 */
void QDocForest::newPrimaryTree(const QString &module)
{
    primaryTree_ = new Tree(module, qdb_);
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
 */
const Node *QDocForest::findNodeForTarget(QStringList &targetPath, const Node *relative,
                                          Node::Genus genus, QString &ref)
{
    int flags = SearchBaseClasses | SearchEnumValues;

    QString entity = targetPath.takeFirst();
    QStringList entityPath = entity.split("::");

    QString target;
    if (!targetPath.isEmpty())
        target = targetPath.takeFirst();

    for (const auto *tree : searchOrder()) {
        const Node *n = tree->findNodeForTarget(entityPath, target, relative, flags, genus, ref);
        if (n)
            return n;
        relative = nullptr;
    }
    return nullptr;
}

/*!
  Print the list of module names ordered according
  to how many successful searches each tree had.
 */
void QDocForest::printLinkCounts(const QString &project)
{
    Location().report(QString("%1: Link Counts").arg(project));
    QMultiMap<int, QString> m;
    for (const auto *tree : searchOrder()) {
        if (tree->linkCount() < 0)
            m.insert(tree->linkCount(), tree->physicalModuleName());
    }
    QString depends = "depends                 +=";
    QString module = project.toLower();
    for (auto it = m.begin(); it != m.end(); ++it) {
        QString line = "  " + it.value();
        if (it.value() != module)
            depends += QLatin1Char(' ') + it.value();
        int pad = 30 - line.length();
        for (int k = 0; k < pad; ++k)
            line += QLatin1Char(' ');
        line += "%1";
        Location().report(line.arg(-(it.key())));
    }
    Location().report("Optimal depends variable:");
    Location().report(depends);
}

/*!
  Print the list of module names ordered according
  to how many successful searches each tree had.
 */
QString QDocForest::getLinkCounts(QStringList &strings, QVector<int> &counts)
{
    QMultiMap<int, QString> m;
    for (const auto *tree : searchOrder()) {
        if (tree->linkCount() < 0)
            m.insert(tree->linkCount(), tree->physicalModuleName());
    }
    QString depends = "depends                 +=";
    QString module = Generator::defaultModuleName().toLower();
    for (auto it = m.begin(); it != m.end(); ++it) {
        if (it.value() != module) {
            counts.append(-(it.key()));
            strings.append(it.value());
            depends += QLatin1Char(' ') + it.value();
        }
    }
    return depends;
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
                                                 Node::Genus genus)
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

QDocDatabase *QDocDatabase::qdocDB_ = nullptr;
NodeMap QDocDatabase::typeNodeMap_;
NodeMultiMap QDocDatabase::obsoleteClasses_;
NodeMultiMap QDocDatabase::classesWithObsoleteMembers_;
NodeMultiMap QDocDatabase::obsoleteQmlTypes_;
NodeMultiMap QDocDatabase::qmlTypesWithObsoleteMembers_;
NodeMultiMap QDocDatabase::cppClasses_;
NodeMultiMap QDocDatabase::qmlBasicTypes_;
NodeMultiMap QDocDatabase::qmlTypes_;
NodeMultiMap QDocDatabase::examples_;
NodeMapMap QDocDatabase::newClassMaps_;
NodeMapMap QDocDatabase::newQmlTypeMaps_;
NodeMultiMapMap QDocDatabase::newSinceMaps_;

/*!
  Constructs the singleton qdoc database object. The singleton
  constructs the \a forest_ object, which is also a singleton.
  \a showInternal_ is normally false. If it is true, qdoc will
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
QDocDatabase::QDocDatabase() : showInternal_(false), singleExec_(false), forest_(this)
{
    // nothing
}

/*!
  Destroys the qdoc database object. This requires destroying
  the forest object, which contains an array of tree pointers.
  Each tree is deleted.
 */
QDocDatabase::~QDocDatabase()
{
    // nothing.
}

/*!
  Creates the singleton. Allows only one instance of the class
  to be created. Returns a pointer to the singleton.
*/
QDocDatabase *QDocDatabase::qdocDB()
{
    if (qdocDB_ == nullptr) {
        qdocDB_ = new QDocDatabase;
        initializeDB();
    }
    return qdocDB_;
}

/*!
  Destroys the singleton.
 */
void QDocDatabase::destroyQdocDB()
{
    if (qdocDB_ != nullptr) {
        delete qdocDB_;
        qdocDB_ = nullptr;
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

  Also calls Node::initialize() to initialize the search goal map.
 */
void QDocDatabase::initializeDB()
{
    Node::initialize();
    typeNodeMap_.insert("accepted", nullptr);
    typeNodeMap_.insert("actionPerformed", nullptr);
    typeNodeMap_.insert("activated", nullptr);
    typeNodeMap_.insert("alias", nullptr);
    typeNodeMap_.insert("anchors", nullptr);
    typeNodeMap_.insert("any", nullptr);
    typeNodeMap_.insert("array", nullptr);
    typeNodeMap_.insert("autoSearch", nullptr);
    typeNodeMap_.insert("axis", nullptr);
    typeNodeMap_.insert("backClicked", nullptr);
    typeNodeMap_.insert("boomTime", nullptr);
    typeNodeMap_.insert("border", nullptr);
    typeNodeMap_.insert("buttonClicked", nullptr);
    typeNodeMap_.insert("callback", nullptr);
    typeNodeMap_.insert("char", nullptr);
    typeNodeMap_.insert("clicked", nullptr);
    typeNodeMap_.insert("close", nullptr);
    typeNodeMap_.insert("closed", nullptr);
    typeNodeMap_.insert("cond", nullptr);
    typeNodeMap_.insert("data", nullptr);
    typeNodeMap_.insert("dataReady", nullptr);
    typeNodeMap_.insert("dateString", nullptr);
    typeNodeMap_.insert("dateTimeString", nullptr);
    typeNodeMap_.insert("datetime", nullptr);
    typeNodeMap_.insert("day", nullptr);
    typeNodeMap_.insert("deactivated", nullptr);
    typeNodeMap_.insert("drag", nullptr);
    typeNodeMap_.insert("easing", nullptr);
    typeNodeMap_.insert("error", nullptr);
    typeNodeMap_.insert("exposure", nullptr);
    typeNodeMap_.insert("fatalError", nullptr);
    typeNodeMap_.insert("fileSelected", nullptr);
    typeNodeMap_.insert("flags", nullptr);
    typeNodeMap_.insert("float", nullptr);
    typeNodeMap_.insert("focus", nullptr);
    typeNodeMap_.insert("focusZone", nullptr);
    typeNodeMap_.insert("format", nullptr);
    typeNodeMap_.insert("framePainted", nullptr);
    typeNodeMap_.insert("from", nullptr);
    typeNodeMap_.insert("frontClicked", nullptr);
    typeNodeMap_.insert("function", nullptr);
    typeNodeMap_.insert("hasOpened", nullptr);
    typeNodeMap_.insert("hovered", nullptr);
    typeNodeMap_.insert("hoveredTitle", nullptr);
    typeNodeMap_.insert("hoveredUrl", nullptr);
    typeNodeMap_.insert("imageCapture", nullptr);
    typeNodeMap_.insert("imageProcessing", nullptr);
    typeNodeMap_.insert("index", nullptr);
    typeNodeMap_.insert("initialized", nullptr);
    typeNodeMap_.insert("isLoaded", nullptr);
    typeNodeMap_.insert("item", nullptr);
    typeNodeMap_.insert("jsdict", nullptr);
    typeNodeMap_.insert("jsobject", nullptr);
    typeNodeMap_.insert("key", nullptr);
    typeNodeMap_.insert("keysequence", nullptr);
    typeNodeMap_.insert("listViewClicked", nullptr);
    typeNodeMap_.insert("loadRequest", nullptr);
    typeNodeMap_.insert("locale", nullptr);
    typeNodeMap_.insert("location", nullptr);
    typeNodeMap_.insert("long", nullptr);
    typeNodeMap_.insert("message", nullptr);
    typeNodeMap_.insert("messageReceived", nullptr);
    typeNodeMap_.insert("mode", nullptr);
    typeNodeMap_.insert("month", nullptr);
    typeNodeMap_.insert("name", nullptr);
    typeNodeMap_.insert("number", nullptr);
    typeNodeMap_.insert("object", nullptr);
    typeNodeMap_.insert("offset", nullptr);
    typeNodeMap_.insert("ok", nullptr);
    typeNodeMap_.insert("openCamera", nullptr);
    typeNodeMap_.insert("openImage", nullptr);
    typeNodeMap_.insert("openVideo", nullptr);
    typeNodeMap_.insert("padding", nullptr);
    typeNodeMap_.insert("parent", nullptr);
    typeNodeMap_.insert("path", nullptr);
    typeNodeMap_.insert("photoModeSelected", nullptr);
    typeNodeMap_.insert("position", nullptr);
    typeNodeMap_.insert("precision", nullptr);
    typeNodeMap_.insert("presetClicked", nullptr);
    typeNodeMap_.insert("preview", nullptr);
    typeNodeMap_.insert("previewSelected", nullptr);
    typeNodeMap_.insert("progress", nullptr);
    typeNodeMap_.insert("puzzleLost", nullptr);
    typeNodeMap_.insert("qmlSignal", nullptr);
    typeNodeMap_.insert("rectangle", nullptr);
    typeNodeMap_.insert("request", nullptr);
    typeNodeMap_.insert("requestId", nullptr);
    typeNodeMap_.insert("section", nullptr);
    typeNodeMap_.insert("selected", nullptr);
    typeNodeMap_.insert("send", nullptr);
    typeNodeMap_.insert("settingsClicked", nullptr);
    typeNodeMap_.insert("shoe", nullptr);
    typeNodeMap_.insert("short", nullptr);
    typeNodeMap_.insert("signed", nullptr);
    typeNodeMap_.insert("sizeChanged", nullptr);
    typeNodeMap_.insert("size_t", nullptr);
    typeNodeMap_.insert("sockaddr", nullptr);
    typeNodeMap_.insert("someOtherSignal", nullptr);
    typeNodeMap_.insert("sourceSize", nullptr);
    typeNodeMap_.insert("startButtonClicked", nullptr);
    typeNodeMap_.insert("state", nullptr);
    typeNodeMap_.insert("std::initializer_list", nullptr);
    typeNodeMap_.insert("std::list", nullptr);
    typeNodeMap_.insert("std::map", nullptr);
    typeNodeMap_.insert("std::pair", nullptr);
    typeNodeMap_.insert("std::string", nullptr);
    typeNodeMap_.insert("std::vector", nullptr);
    typeNodeMap_.insert("stringlist", nullptr);
    typeNodeMap_.insert("swapPlayers", nullptr);
    typeNodeMap_.insert("symbol", nullptr);
    typeNodeMap_.insert("t", nullptr);
    typeNodeMap_.insert("T", nullptr);
    typeNodeMap_.insert("tagChanged", nullptr);
    typeNodeMap_.insert("timeString", nullptr);
    typeNodeMap_.insert("timeout", nullptr);
    typeNodeMap_.insert("to", nullptr);
    typeNodeMap_.insert("toggled", nullptr);
    typeNodeMap_.insert("type", nullptr);
    typeNodeMap_.insert("unsigned", nullptr);
    typeNodeMap_.insert("urllist", nullptr);
    typeNodeMap_.insert("va_list", nullptr);
    typeNodeMap_.insert("value", nullptr);
    typeNodeMap_.insert("valueEmitted", nullptr);
    typeNodeMap_.insert("videoFramePainted", nullptr);
    typeNodeMap_.insert("videoModeSelected", nullptr);
    typeNodeMap_.insert("videoRecorder", nullptr);
    typeNodeMap_.insert("void", nullptr);
    typeNodeMap_.insert("volatile", nullptr);
    typeNodeMap_.insert("wchar_t", nullptr);
    typeNodeMap_.insert("x", nullptr);
    typeNodeMap_.insert("y", nullptr);
    typeNodeMap_.insert("zoom", nullptr);
    typeNodeMap_.insert("zoomTo", nullptr);
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

/*!
  \fn const CNMap &QDocDatabase::jsModules()
  Returns a const reference to the collection of all
  JovaScript module nodes in the primary tree.
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

/*! \fn CollectionNode *QDocDatabase::findQmlModule(const QString &name, bool javaScript)
  Find the QML module node named \a name and return a pointer
  to it. If a matching node is not found, add a new QML module
  node named \a name and return a pointer to that one.

  If \a javaScript is set, the return collection must be a
  JavaScript module.

  If a new QML or JavaScript module node is added, its parent
  is the tree root, and the new node is marked \e{not seen}.
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

/*! \fn CollectionNode *QDocDatabase::addJsModule(const QString &name)
  Looks up the JavaScript module named \a name in the primary
  tree. If a match is found, a pointer to the node is returned.
  Otherwise, a new JavaScript module node named \a name is
  created and inserted into the collection, and the pointer to
  that node is returned.
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

/*! \fn Collection *QDocDatabase::addToJsModule(const QString &name, Node *node)
  Looks up the JavaScript module named \a name. If it isn't there,
  create it. Then append \a node to the JavaScript module's member
  list. The parent of \a node is not changed by this function.
 */

/*!
  Looks up the QML type node identified by the qualified Qml
  type \a name and returns a pointer to the QML type node.
 */
QmlTypeNode *QDocDatabase::findQmlType(const QString &name)
{
    QmlTypeNode *qcn = forest_.lookupQmlType(name);
    if (qcn)
        return qcn;
    return nullptr;
}

/*!
  Looks up the QML type node identified by the Qml module id
  \a qmid and QML type \a name and returns a pointer to the
  QML type node. The key is \a qmid + "::" + \a name.

  If the QML module id is empty, it looks up the QML type by
  \a name only.
 */
QmlTypeNode *QDocDatabase::findQmlType(const QString &qmid, const QString &name)
{
    if (!qmid.isEmpty()) {
        QString t = qmid + "::" + name;
        QmlTypeNode *qcn = forest_.lookupQmlType(t);
        if (qcn)
            return qcn;
    }

    QStringList path(name);
    Node *n = forest_.findNodeByNameAndType(path, &Node::isQmlType);
    if (n && (n->isQmlType() || n->isJsType()))
        return static_cast<QmlTypeNode *>(n);
    return nullptr;
}

/*!
  Looks up the QML basic type node identified by the Qml module id
  \a qmid and QML basic type \a name and returns a pointer to the
  QML basic type node. The key is \a qmid + "::" + \a name.

  If the QML module id is empty, it looks up the QML basic type by
  \a name only.
 */
Aggregate *QDocDatabase::findQmlBasicType(const QString &qmid, const QString &name)
{
    if (!qmid.isEmpty()) {
        QString t = qmid + "::" + name;
        Aggregate *a = forest_.lookupQmlBasicType(t);
        if (a)
            return a;
    }

    QStringList path(name);
    Node *n = forest_.findNodeByNameAndType(path, &Node::isQmlBasicType);
    if (n && n->isQmlBasicType())
        return static_cast<Aggregate *>(n);
    return nullptr;
}

/*!
  Looks up the QML type node identified by the Qml module id
  constructed from the strings in the \a import record and the
  QML type \a name and returns a pointer to the QML type node.
  If a QML type node is not found, 0 is returned.
 */
QmlTypeNode *QDocDatabase::findQmlType(const ImportRec &import, const QString &name)
{
    if (!import.isEmpty()) {
        QStringList dotSplit;
        dotSplit = name.split(QLatin1Char('.'));
        QString qmName;
        if (import.importUri_.isEmpty())
            qmName = import.name_;
        else
            qmName = import.importUri_;
        for (int i = 0; i < dotSplit.size(); ++i) {
            QString qualifiedName = qmName + "::" + dotSplit[i];
            QmlTypeNode *qcn = forest_.lookupQmlType(qualifiedName);
            if (qcn)
                return qcn;
        }
    }
    return nullptr;
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
    Tree *t = forest_.firstTree();
    while (t) {
        findAllClasses(t->root());
        findAllFunctions(t->root());
        findAllObsoleteThings(t->root());
        findAllLegaleseTexts(t->root());
        findAllSince(t->root());
        findAllAttributions(t->root());
        t->setTreeHasBeenAnalyzed();
        t = forest_.nextTree();
    }
    resolveNamespaces();
}

/*!
  This function calls \a func for each tree in the forest,
  but only if Tree::treeHasBeenAnalyzed() returns false for
  the tree. In this way, when running qdoc in \e singleExec
  mode, each tree is analyzed in turn, and its classes and
  types are added to the appropriate node maps.
 */
void QDocDatabase::processForest(void (QDocDatabase::*func)(Aggregate *))
{
    Tree *t = forest_.firstTree();
    while (t) {
        if (!t->treeHasBeenAnalyzed()) {
            (this->*(func))(t->root());
        }
        t = forest_.nextTree();
    }
}

/*!
  Constructs the collection of legalese texts, if it has not
  already been constructed, and returns a reference to it.
 */
TextToNodeMap &QDocDatabase::getLegaleseTexts()
{
    if (legaleseTexts_.isEmpty())
        processForest(&QDocDatabase::findAllLegaleseTexts);
    return legaleseTexts_;
}

/*!
  Construct the data structures for obsolete things, if they
  have not already been constructed. Returns a reference to
  the map of C++ classes with obsolete members.
 */
NodeMultiMap &QDocDatabase::getClassesWithObsoleteMembers()
{
    if (obsoleteClasses_.isEmpty() && obsoleteQmlTypes_.isEmpty())
        processForest(&QDocDatabase::findAllObsoleteThings);
    return classesWithObsoleteMembers_;
}

/*!
  Construct the data structures for obsolete things, if they
  have not already been constructed. Returns a reference to
  the map of obsolete QML types.
 */
NodeMultiMap &QDocDatabase::getObsoleteQmlTypes()
{
    if (obsoleteClasses_.isEmpty() && obsoleteQmlTypes_.isEmpty())
        processForest(&QDocDatabase::findAllObsoleteThings);
    return obsoleteQmlTypes_;
}

/*!
  Construct the data structures for obsolete things, if they
  have not already been constructed. Returns a reference to
  the map of QML types with obsolete members.
 */
NodeMultiMap &QDocDatabase::getQmlTypesWithObsoleteMembers()
{
    if (obsoleteClasses_.isEmpty() && obsoleteQmlTypes_.isEmpty())
        processForest(&QDocDatabase::findAllObsoleteThings);
    return qmlTypesWithObsoleteMembers_;
}

/*! \fn NodeMultiMap &QDocDatabase::getNamespaces()
  Returns a reference to the map of all namespace nodes.
  This function must not be called in the -prepare phase.
 */

/*!
  Construct the data structures for QML basic types, if they
  have not already been constructed. Returns a reference to
  the map of QML basic types.
 */
NodeMultiMap &QDocDatabase::getQmlBasicTypes()
{
    if (cppClasses_.isEmpty() && qmlBasicTypes_.isEmpty())
        processForest(&QDocDatabase::findAllClasses);
    return qmlBasicTypes_;
}

/*!
  Construct the data structures for QML types, if they
  have not already been constructed. Returns a reference to
  the multimap of QML types.
 */
NodeMultiMap &QDocDatabase::getQmlTypes()
{
    if (cppClasses_.isEmpty() && qmlTypes_.isEmpty())
        processForest(&QDocDatabase::findAllClasses);
    return qmlTypes_;
}

/*!
  Construct the data structures for examples, if they
  have not already been constructed. Returns a reference to
  the multimap of example nodes.
 */
NodeMultiMap &QDocDatabase::getExamples()
{
    if (cppClasses_.isEmpty() && examples_.isEmpty())
        processForest(&QDocDatabase::findAllClasses);
    return examples_;
}

/*!
  Construct the data structures for attributions, if they
  have not already been constructed. Returns a reference to
  the multimap of attribution nodes.
 */
NodeMultiMap &QDocDatabase::getAttributions()
{
    if (attributions_.isEmpty())
        processForest(&QDocDatabase::findAllAttributions);
    return attributions_;
}

/*!
  Construct the data structures for obsolete things, if they
  have not already been constructed. Returns a reference to
  the map of obsolete C++ clases.
 */
NodeMultiMap &QDocDatabase::getObsoleteClasses()
{
    if (obsoleteClasses_.isEmpty() && obsoleteQmlTypes_.isEmpty())
        processForest(&QDocDatabase::findAllObsoleteThings);
    return obsoleteClasses_;
}

/*!
  Construct the C++ class data structures, if they have not
  already been constructed. Returns a reference to the map
  of all C++ classes.
 */
NodeMultiMap &QDocDatabase::getCppClasses()
{
    if (cppClasses_.isEmpty() && qmlTypes_.isEmpty())
        processForest(&QDocDatabase::findAllClasses);
    return cppClasses_;
}

/*!
  Construct the function index data structure and return it.
  This data structure is used to output the function index page.
 */
NodeMapMap &QDocDatabase::getFunctionIndex()
{
    if (functionIndex_.isEmpty())
        processForest(&QDocDatabase::findAllFunctions);
    return functionIndex_;
}

/*!
  Finds all the nodes containing legalese text and puts them
  in a map.
 */
void QDocDatabase::findAllLegaleseTexts(Aggregate *node)
{
    for (auto it = node->constBegin(); it != node->constEnd(); ++it) {
        if (!(*it)->isPrivate()) {
            if (!(*it)->doc().legaleseText().isEmpty())
                legaleseTexts_.insert((*it)->doc().legaleseText(), *it);
            if ((*it)->isAggregate())
                findAllLegaleseTexts(static_cast<Aggregate *>(*it));
        }
    }
}

/*!
  \fn void QDocDatabase::findAllObsoleteThings(Aggregate *node)

  Finds all nodes with status = Obsolete and sorts them into
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
const NodeMap &QDocDatabase::getClassMap(const QString &key)
{
    if (newSinceMaps_.isEmpty() && newClassMaps_.isEmpty() && newQmlTypeMaps_.isEmpty())
        processForest(&QDocDatabase::findAllSince);
    auto it = newClassMaps_.constFind(key);
    if (it != newClassMaps_.constEnd())
        return it.value();
    return emptyNodeMap_;
}

/*!
  Find the \a key in the map of new QML type maps, and return a
  reference to the value, which is a NodeMap. If the \a key is not
  found, return a reference to an empty NodeMap.
 */
const NodeMap &QDocDatabase::getQmlTypeMap(const QString &key)
{
    if (newSinceMaps_.isEmpty() && newClassMaps_.isEmpty() && newQmlTypeMaps_.isEmpty())
        processForest(&QDocDatabase::findAllSince);
    auto it = newQmlTypeMaps_.constFind(key);
    if (it != newQmlTypeMaps_.constEnd())
        return it.value();
    return emptyNodeMap_;
}

/*!
  Find the \a key in the map of new \e {since} maps, and return
  a reference to the value, which is a NodeMultiMap. If \a key
  is not found, return a reference to an empty NodeMultiMap.
 */
const NodeMap &QDocDatabase::getSinceMap(const QString &key)
{
    if (newSinceMaps_.isEmpty() && newClassMaps_.isEmpty() && newQmlTypeMaps_.isEmpty())
        processForest(&QDocDatabase::findAllSince);
    auto it = newSinceMaps_.constFind(key);
    if (it != newSinceMaps_.constEnd())
        return it.value();
    return emptyNodeMultiMap_;
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
        primaryTreeRoot()->normalizeOverloads();
        primaryTree()->markDontDocumentNodes();
        primaryTree()->removePrivateAndInternalBases(primaryTreeRoot());
        primaryTree()->resolveProperties();
        primaryTreeRoot()->markUndocumentedChildrenInternal();
        primaryTreeRoot()->resolveQmlInheritance();
        primaryTree()->resolveTargets(primaryTreeRoot());
        primaryTree()->resolveCppToQmlLinks();
        primaryTree()->resolveUsingClauses();
    }
    if (config.singleExec() && config.generating()) {
        primaryTree()->resolveBaseClasses(primaryTreeRoot());
        primaryTree()->resolvePropertyOverriddenFromPtrs(primaryTreeRoot());
        primaryTreeRoot()->resolveQmlInheritance();
        primaryTree()->resolveCppToQmlLinks();
        primaryTree()->resolveUsingClauses();
    }
    if (config.generating()) {
        resolveNamespaces();
        resolveProxies();
        resolveBaseClasses();
    }
    if (config.dualExec())
        QDocIndexFiles::destroyQDocIndexFiles();
}

void QDocDatabase::resolveBaseClasses()
{
    Tree *t = forest_.firstTree();
    while (t) {
        t->resolveBaseClasses(t->root());
        t = forest_.nextTree();
    }
}

/*!
  Returns a reference to the namespace map. Constructs the
  namespace map if it hasn't been constructed yet.
 */
NodeMultiMap &QDocDatabase::getNamespaces()
{
    resolveNamespaces();
    return namespaceIndex_;
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
    if (!namespaceIndex_.isEmpty())
        return;
    NodeMultiMap namespaceMultimap;
    Tree *t = forest_.firstTree();
    while (t) {
        t->root()->findAllNamespaces(namespaceMultimap);
        t = forest_.nextTree();
    }
    const QList<QString> keys = namespaceMultimap.uniqueKeys();
    for (const QString &key : keys) {
        NamespaceNode *ns = nullptr;
        NamespaceNode *somewhere = nullptr;
        const NodeList namespaces = namespaceMultimap.values(key);
        int count = namespaceMultimap.remove(key);
        if (count > 0) {
            for (auto *node : namespaces) {
                ns = static_cast<NamespaceNode *>(node);
                if (ns->isDocumentedHere())
                    break;
                else if (ns->hadDoc())
                    somewhere = ns;
                ns = nullptr;
            }
            if (ns) {
                for (auto *node : namespaces) {
                    NamespaceNode *NS = static_cast<NamespaceNode *>(node);
                    if (NS->hadDoc() && NS != ns) {
                        ns->doc().location().warning(
                                tr("Namespace %1 documented more than once").arg(NS->name()));
                        NS->doc().location().warning(tr("...also seen here"));
                    }
                }

            } else if (somewhere == nullptr) {
                for (auto *node : namespaces) {
                    NamespaceNode *NS = static_cast<NamespaceNode *>(node);
                    NS->reportDocumentedChildrenInUndocumentedNamespace();
                }
            }
            if (somewhere) {
                for (auto *node : namespaces) {
                    NamespaceNode *NS = static_cast<NamespaceNode *>(node);
                    if (NS != somewhere)
                        NS->setDocNode(somewhere);
                }
            }
        }
        /*
          If there are multiple namespace nodes with the same
          name and one of them will be the reference page for
          the namespace, include all the nodes in the public
          API of the namespace in the single namespace node
          that will generate the namespace reference page for
          the namespace.
         */
        if (ns && count > 1) {
            for (auto *node : namespaces) {
                auto *nameSpaceNode = static_cast<NamespaceNode *>(node);
                if (nameSpaceNode != ns) {
                    for (auto it = nameSpaceNode->constBegin(); it != nameSpaceNode->constEnd();
                         ++it) {
                        Node *N = *it;
                        if (N && N->isPublic() && !N->isInternal())
                            ns->includeChild(N);
                    }
                }
            }
        }
        if (ns == nullptr)
            ns = static_cast<NamespaceNode *>(namespaces.at(0));
        namespaceIndex_.insert(ns->name(), ns);
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
    Tree *t = forest_.firstTree();
    t = forest_.nextTree();
    while (t) {
        const NodeList &proxies = t->proxies();
        if (!proxies.isEmpty()) {
            for (auto *node : proxies) {
                ProxyNode *pn = static_cast<ProxyNode *>(node);
                if (pn->count() > 0) {
                    Aggregate *aggregate = primaryTree()->findAggregate(pn->name());
                    if (aggregate != nullptr)
                        aggregate->appendToRelatedByProxy(pn->childNodes());
                }
            }
        }
        t = forest_.nextTree();
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
                                                   Node::Genus genus)
{
    QString signature;
    QString function = target;
    int length = target.length();
    if (function.endsWith("()"))
        function.chop(2);
    if (function.endsWith(QChar(')'))) {
        int position = function.lastIndexOf(QChar('('));
        signature = function.mid(position + 1, length - position - 2);
        function = function.left(position);
    }
    QStringList path = function.split("::");
    return forest_.findFunctionNode(path, Parameters(signature), relative, genus);
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
const Node *QDocDatabase::findTypeNode(const QString &type, const Node *relative, Node::Genus genus)
{
    QStringList path = type.split("::");
    if ((path.size() == 1) && (path.at(0)[0].isLower() || path.at(0) == QString("T"))) {
        auto it = typeNodeMap_.find(path.at(0));
        if (it != typeNodeMap_.end())
            return it.value();
    }
    return forest_.findTypeNode(path, relative, genus);
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
            const Node *n = tree->findNode(path, relative, flags, Node::DontCare);
            if (n)
                return n;
            relative = nullptr;
        }
        node = findPageNodeByTitle(target);
    }
    return node;
}

/*!
  Generates a tag file and writes it to \a name.
 */
void QDocDatabase::generateTagFile(const QString &name, Generator *g)
{
    if (!name.isEmpty()) {
        QDocTagFiles::qdocTagFiles()->generateTagFile(name, g);
        QDocTagFiles::destroyQDocTagFiles();
    }
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
            qDebug() << "This index file is already in memory:" << file;
    }
    QDocIndexFiles::qdocIndexFiles()->readIndexes(filesToRead);
}

/*!
  Generates a qdoc index file and write it to \a fileName. The
  index file is generated with the parameters \a url and \a title,
  using the generator \a g.
 */
void QDocDatabase::generateIndex(const QString &fileName, const QString &url, const QString &title,
                                 Generator *g)
{
    QString t = fileName.mid(fileName.lastIndexOf(QChar('/')) + 1);
    primaryTree()->setIndexFileName(t);
    QDocIndexFiles::qdocIndexFiles()->generateIndex(fileName, url, title, g);
    QDocIndexFiles::destroyQDocIndexFiles();
}

/*!
  Find a node of the specified \a type that is reached with
  the specified \a path qualified with the name of one of the
  open namespaces (might not be any open ones). If the node
  is found in an open namespace, prefix \a path with the name
  of the open namespace and "::" and return a pointer to the
  node. Othewrwise return 0.

  This function only searches in the current primary tree.
 */
Node *QDocDatabase::findNodeInOpenNamespace(QStringList &path, bool (Node::*isMatch)() const)
{
    if (path.isEmpty())
        return nullptr;
    Node *n = nullptr;
    if (!openNamespaces_.isEmpty()) {
        const auto &openNamespaces = openNamespaces_;
        for (const QString &t : openNamespaces) {
            QStringList p;
            if (t != path[0])
                p = t.split("::") + path;
            else
                p = path;
            n = primaryTree()->findNodeByNameAndType(p, isMatch);
            if (n) {
                path = p;
                break;
            }
        }
    }
    return n;
}

/*!
  Finds all the collection nodes of the specified \a type
  and merges them into the collection node map \a cnm. Nodes
  that match the \a relative node are not included.
 */
void QDocDatabase::mergeCollections(Node::NodeType type, CNMap &cnm, const Node *relative)
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
    QRegExp singleDigit("\\b([0-9])\\b");
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
                        // Allow multiple (major) versions of QML/JS modules
                        if ((n->isQmlModule() || n->isJsModule())
                            && n->logicalModuleIdentifier() != value->logicalModuleIdentifier()) {
                            if (value->wasSeen() && value != relative
                                && !value->members().isEmpty())
                                cnm.insert(value->fullTitle().toLower(), value);
                            continue;
                        }
                        for (Node *t : value->members())
                            n->addMember(t);
                    }
                }
            }
            if (!n->members().isEmpty()) {
                QString sortKey = n->fullTitle().toLower();
                if (sortKey.startsWith("the "))
                    sortKey.remove(0, 4);
                sortKey.replace(singleDigit, "0\\1");
                cnm.insert(sortKey, n);
            }
        }
    }
}

/*!
  Finds all the collection nodes with the same name
  and type as \a c and merges their members into the
  members list of \a c.

  For QML and JS modules, only nodes with matching
  module identifiers are merged to avoid merging
  modules with different (major) versions.
 */
void QDocDatabase::mergeCollections(CollectionNode *c)
{
    for (auto *tree : searchOrder()) {
        CollectionNode *cn = tree->getCollection(c->name(), c->nodeType());
        if (cn && cn != c) {
            if ((cn->isQmlModule() || cn->isJsModule())
                && cn->logicalModuleIdentifier() != c->logicalModuleIdentifier())
                continue;
            for (auto *node : cn->members())
                c->addMember(node);
        }
    }
}

/*!
  Searches for the node that matches the path in \a atom. The
  \a relative node is used if the first leg of the path is
  empty, i.e. if the path begins with a hashtag. The function
  also sets \a ref if there remains an unused leg in the path
  after the node is found. The node is returned as well as the
  \a ref. If the returned node pointer is null, \a ref is not
  valid.
 */
const Node *QDocDatabase::findNodeForAtom(const Atom *a, const Node *relative, QString &ref)
{
    const Node *node = nullptr;

    Atom *atom = const_cast<Atom *>(a);
    QStringList targetPath = atom->string().split(QLatin1Char('#'));
    QString first = targetPath.first().trimmed();

    Tree *domain = nullptr;
    Node::Genus genus = Node::DontCare;
    // Reserved for future use
    // Node::NodeType goal = Node::NoType;

    if (atom->isLinkAtom()) {
        domain = atom->domain();
        genus = atom->genus();
        // Reserved for future use
        // goal = atom->goal();
    }

    if (first.isEmpty())
        node = relative; // search for a target on the current page.
    else if (domain) {
        if (first.endsWith(".html"))
            node = domain->findNodeByNameAndType(QStringList(first), &Node::isPageNode);
        else if (first.endsWith(QChar(')'))) {
            QString signature;
            QString function = first;
            int length = first.length();
            if (function.endsWith("()"))
                function.chop(2);
            if (function.endsWith(QChar(')'))) {
                int position = function.lastIndexOf(QChar('('));
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

QT_END_NAMESPACE
