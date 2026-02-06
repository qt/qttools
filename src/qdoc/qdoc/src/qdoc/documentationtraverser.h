// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOCUMENTATIONTRAVERSER_H
#define DOCUMENTATIONTRAVERSER_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class IDocumentationHandler;
class Node;

class DocumentationTraverser
{
public:
    DocumentationTraverser() = default;
    void traverse(Node *root, IDocumentationHandler &handler);

private:
    void traverseNode(Node *node, IDocumentationHandler &handler);
    [[nodiscard]] bool shouldSkip(const Node *node) const;
};

QT_END_NAMESPACE

#endif // DOCUMENTATIONTRAVERSER_H

