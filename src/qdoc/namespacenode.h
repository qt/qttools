/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef NAMESPACENODE_H
#define NAMESPACENODE_H

#include "aggregate.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Tree;

class NamespaceNode : public Aggregate
{
public:
    NamespaceNode(Aggregate *parent, const QString &name) : Aggregate(Namespace, parent, name) {}
    ~NamespaceNode() override = default;
    [[nodiscard]] Tree *tree() const override { return (parent() ? parent()->tree() : m_tree); }

    [[nodiscard]] bool isFirstClassAggregate() const override { return true; }
    [[nodiscard]] bool isRelatableType() const override { return true; }
    [[nodiscard]] bool wasSeen() const override { return m_seen; }
    void markSeen() { m_seen = true; }
    void setTree(Tree *t) { m_tree = t; }
    [[nodiscard]] const NodeList &includedChildren() const;
    void includeChild(Node *child);
    void setWhereDocumented(const QString &t) { m_whereDocumented = t; }
    [[nodiscard]] bool isDocumentedHere() const;
    [[nodiscard]] bool hasDocumentedChildren() const;
    void reportDocumentedChildrenInUndocumentedNamespace() const;
    [[nodiscard]] bool docMustBeGenerated() const override;
    void setDocNode(NamespaceNode *ns) { m_docNode = ns; }
    [[nodiscard]] NamespaceNode *docNode() const { return m_docNode; }

private:
    bool m_seen { false };
    Tree *m_tree { nullptr };
    QString m_whereDocumented {};
    NamespaceNode *m_docNode { nullptr };
    NodeList m_includedChildren {};
};

QT_END_NAMESPACE

#endif // NAMESPACENODE_H
