// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "documentationtraverser.h"

#include "aggregate.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "documentationhandler.h"
#include "inclusionfilter.h"
#include "node.h"
#include "pagenode.h"
#include "qmltypenode.h"
#include "tree.h"
#include "utilities.h"

#include <QtCore/qfileinfo.h>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

/*!
    \class DocumentationTraverser
    \internal
    \brief Traverses the node tree and dispatches to a handler for documentation
           generation.

    DocumentationTraverser encapsulates the tree traversal logic that was
    previously embedded in Generator::generateDocumentation(). It handles:

    \list
        \li Filtering nodes (URL nodes, index nodes, excluded by policy,
            external pages).
        \li Looking up the appropriate CodeMarker for each node.
        \li Dispatching by node type to the handler's generate* methods.
        \li Recursively processing child nodes.
    \endlist

    The actual content generation and file lifecycle are delegated to an
    DocumentationHandler, enabling different generators to share traversal
    logic while implementing their own output strategies.

    \section1 Usage

    \code
    DocumentationTraverser traverser;
    MyDocumentationHandler handler(writer, resolver);
    traverser.traverse(rootNode, handler);
    \endcode

    \sa DocumentationHandler, Generator, TemplateGenerator
*/

/*!
    \internal
    Traverses the node tree starting from \a root, dispatching to \a handler
    for each documentable node.
*/
void DocumentationTraverser::traverse(Node *root, DocumentationHandler &handler)
{
    traverseNode(root, handler);
}

/*!
    \internal
    Processes a single node and recursively processes its children.

    This method handles:
    \list
        \li Filtering nodes that should be skipped.
        \li Looking up the CodeMarker for syntax highlighting.
        \li Dispatching by node type to the handler.
        \li Managing file lifecycle via handler callbacks.
        \li Recursively processing child nodes.
    \endlist
*/
void DocumentationTraverser::traverseNode(Node *node, DocumentationHandler &handler)
{
    if (shouldSkip(node))
        return;

    CodeMarker *marker = CodeMarker::markerForFileName(node->location().filePath());

    if (node->parent() != nullptr) {
        if (node->isCollectionNode()) {
            /*
              A collection node collects: groups, C++ modules, or QML
              modules. Testing for a CollectionNode must be done
              before testing for a TextPageNode because a
              CollectionNode is a PageNode at this point.

              Don't output an HTML page for the collection node unless
              the \group, \module, or \qmlmodule command was actually
              seen by qdoc in the qdoc comment for the node.

              A key prerequisite in this case is the call to
              mergeCollections(cn). We must determine whether this
              group, module or QML module has members in other
              modules. We know at this point that cn's members list
              contains only members in the current module. Therefore,
              before outputting the page for cn, we must search for
              members of cn in the other modules and add them to the
              members list.
            */
            auto *cn = static_cast<CollectionNode *>(node);
            if (cn->wasSeen()) {
                handler.mergeCollections(cn);
                handler.beginDocument(handler.fileName(node));
                handler.generateCollectionNode(cn, marker);
                handler.endDocument();
            } else if (cn->isGenericCollection()) {
                // Currently used only for the module's related orphans page
                // but can be generalized for other kinds of collections if
                // other use cases pop up.
                QString name = cn->name().toLower();
                name.replace(QChar(' '), QString("-"));
                QString filename = cn->tree()->physicalModuleName() + "-" + name + "."
                        + QFileInfo(handler.fileName(node)).suffix();
                handler.beginDocument(filename);
                handler.generateGenericCollectionPage(cn, marker);
                handler.endDocument();
            }
        } else if (node->isTextPageNode()) {
            handler.beginDocument(handler.fileName(node));
            handler.generatePageNode(static_cast<PageNode *>(node), marker);
            handler.endDocument();
        } else if (node->isAggregate()) {
            if ((node->isClassNode() || node->isHeader() || node->isNamespace())
                && node->docMustBeGenerated()) {
                handler.beginDocument(handler.fileName(node));
                handler.generateCppReferencePage(static_cast<Aggregate *>(node), marker);
                handler.endDocument();
            } else if (node->isQmlType()) {
                handler.beginDocument(handler.fileName(node));
                auto *qcn = static_cast<QmlTypeNode *>(node);
                handler.generateQmlTypePage(qcn, marker);
                handler.endDocument();
            } else if (node->isProxyNode()) {
                handler.beginDocument(handler.fileName(node));
                handler.generateProxyPage(static_cast<Aggregate *>(node), marker);
                handler.endDocument();
            }
        }
    }

    // Recursively process children
    if (node->isAggregate()) {
        auto *aggregate = static_cast<Aggregate *>(node);
        const NodeList &children = aggregate->childNodes();
        for (auto *child : children) {
            if (child->isPageNode()) {
                traverseNode(child, handler);
            } else if (!node->parent() && child->isInAPI() && !child->isRelatedNonmember()
                       && !child->doc().isAutoGenerated()) {
                // Warn if there are documented non-page-generating nodes in the root namespace
                child->location().warning(
                        u"No documentation generated for %1 '%2' in global scope."_s
                                .arg(child->nodeTypeString(), child->name()),
                        u"Maybe you forgot to use the '\\relates' command?"_s);
                child->setStatus(Status::DontDocument);
            } else if (child->isQmlModule() && !child->wasSeen()) {
                // An undocumented QML module that was constructed as a placeholder
                auto *qmlModule = static_cast<CollectionNode *>(child);
                for (const auto *member : qmlModule->members()) {
                    member->location().warning(
                            u"Undocumented QML module '%1' referred by type '%2' or its members"_s
                                    .arg(qmlModule->name(), member->name()),
                            u"Maybe you forgot to document '\\qmlmodule %1'?"_s
                                    .arg(qmlModule->name()));
                }
            } else if (child->isQmlType() && !child->hasDoc()) {
                // A placeholder QML type with incorrect module identifier
                auto *qmlType = static_cast<QmlTypeNode *>(child);
                if (auto qmid = qmlType->logicalModuleName(); !qmid.isEmpty())
                    qmlType->location().warning(
                            u"No such type '%1' in QML module '%2'"_s.arg(qmlType->name(), qmid));
            }
        }
    }
}

/*!
    \internal
    Returns true if the node should be skipped during traversal.

    Nodes are skipped if they:
    \list
        \li Have a URL (external documentation).
        \li Are index nodes (from loaded .index files).
        \li Are excluded by the inclusion policy.
        \li Are external pages.
    \endlist
*/
bool DocumentationTraverser::shouldSkip(const Node *node) const
{
    if (!node->url().isNull())
        return true;
    if (node->isIndexNode())
        return true;

    const InclusionPolicy policy = Config::instance().createInclusionPolicy();
    const NodeContext context = node->createContext();
    if (!InclusionFilter::isIncluded(policy, context))
        return true;

    if (node->isExternalPage())
        return true;

    return false;
}

QT_END_NAMESPACE

