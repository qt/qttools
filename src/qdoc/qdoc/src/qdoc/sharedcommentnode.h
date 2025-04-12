// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SHAREDCOMMENTNODE_H
#define SHAREDCOMMENTNODE_H

#include "genustypes.h"
#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class QmlTypeNode;

class SharedCommentNode : public Node
{
public:
    explicit SharedCommentNode(Node *node) : Node(NodeType::SharedComment, node->parent(), QString())
    {
        m_collective.reserve(1);
        append(node);
    }
    SharedCommentNode(QmlTypeNode *parent, int count, QString &group);
    ~SharedCommentNode() override { m_collective.clear(); }

    [[nodiscard]] bool isPropertyGroup() const override
    {
        return !name().isEmpty() && !m_collective.isEmpty() && (m_collective.at(0)->isQmlProperty());
    }
    [[nodiscard]] qsizetype count() const { return m_collective.size(); }
    void append(Node *node)
    {
        m_collective.append(node);
        node->setSharedCommentNode(this);
        setGenus(node->genus());
    }
    void sort() { std::sort(m_collective.begin(), m_collective.end(), Node::nodeNameLessThan); }
    [[nodiscard]] const QList<Node *> &collective() const { return m_collective; }
    void setOverloadFlags();
    void setRelatedNonmember(bool value) override;
    Node *clone(Aggregate *parent) override;

private:
    QList<Node *> m_collective {};
};

QT_END_NAMESPACE

#endif // SHAREDCOMMENTNODE_H
