/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef SHAREDCOMMENTNODE_H
#define SHAREDCOMMENTNODE_H

#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class QmlTypeNode;

class SharedCommentNode : public Node
{
public:
    explicit SharedCommentNode(Node *node) : Node(Node::SharedComment, node->parent(), QString())
    {
        m_collective.reserve(1);
        append(node);
    }
    SharedCommentNode(QmlTypeNode *parent, int count, QString &group);
    ~SharedCommentNode() override { m_collective.clear(); }

    [[nodiscard]] bool isPropertyGroup() const override
    {
        return !name().isEmpty() && !m_collective.isEmpty()
                && (m_collective.at(0)->isQmlProperty() || m_collective.at(0)->isJsProperty());
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
