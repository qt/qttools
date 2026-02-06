// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef IDOCUMENTATIONHANDLER_H
#define IDOCUMENTATIONHANDLER_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class CodeMarker;
class CollectionNode;
class Location;
class Node;
class PageNode;
class QmlTypeNode;

/*!
    \class IDocumentationHandler
    \internal
    \brief Interface handling documentation generation during tree traversal.

    IDocumentationHandler defines the callbacks that DocumentationTraverser
    invokes as it walks the node tree. Implementations control file lifecycle
    and content generation while the traverser handles filtering, dispatch, and
    recursion.

    This separation enables composition over inheritance: both legacy Generator
    and TemplateGenerator (new) can use the same traversal logic with different
    handler implementations.

    \sa DocumentationTraverser
*/
class IDocumentationHandler
{
public:
    virtual ~IDocumentationHandler() = default;

    // === File Lifecycle ===
    virtual void beginDocument(const QString &fileName) = 0;

    virtual void endDocument() = 0;

    // === Filename Computation ===
    [[nodiscard]] virtual QString fileName(const Node *node) const = 0;

    // === Content Generation ===
    virtual void generateCollectionNode(CollectionNode *cn, CodeMarker *marker) = 0;
    virtual void generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker) = 0;
    virtual void generatePageNode(PageNode *pn, CodeMarker *marker) = 0;
    virtual void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) = 0;
    virtual void generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker) = 0;
    virtual void generateProxyPage(Aggregate *aggregate, CodeMarker *marker) = 0;

    // === Database Operations ===
    virtual void mergeCollections(CollectionNode *cn) = 0;
};

QT_END_NAMESPACE

#endif // IDOCUMENTATIONHANDLER_H

