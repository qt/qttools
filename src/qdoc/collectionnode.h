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

#ifndef COLLECTIONNODE_H
#define COLLECTIONNODE_H

#include "pagenode.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class CollectionNode : public PageNode
{
public:
    CollectionNode(NodeType type, Aggregate *parent, const QString &name)
        : PageNode(type, parent, name)
    {
    }

    [[nodiscard]] bool isCollectionNode() const override { return true; }
    [[nodiscard]] QString qtVariable() const override { return m_qtVariable; }
    void setQtVariable(const QString &v) override { m_qtVariable = v; }
    [[nodiscard]] QString qtCMakeComponent() const override { return m_qtCMakeComponent; }
    void setQtCMakeComponent(const QString &target) override { m_qtCMakeComponent = target; }
    void addMember(Node *node) override;
    [[nodiscard]] bool hasNamespaces() const override;
    [[nodiscard]] bool hasClasses() const override;
    void getMemberNamespaces(NodeMap &out) override;
    void getMemberClasses(NodeMap &out) const override;
    [[nodiscard]] bool wasSeen() const override { return m_seen; }

    [[nodiscard]] QString fullTitle() const override { return title(); }
    [[nodiscard]] QString logicalModuleName() const override { return m_logicalModuleName; }
    [[nodiscard]] QString logicalModuleVersion() const override;
    [[nodiscard]] QString logicalModuleIdentifier() const override
    {
        return m_logicalModuleName + m_logicalModuleVersionMajor;
    }
    void setLogicalModuleInfo(const QString &arg) override;
    void setLogicalModuleInfo(const QStringList &info) override;

    [[nodiscard]] const NodeList &members() const { return m_members; }

    void markSeen() { m_seen = true; }
    void markNotSeen() { m_seen = false; }

private:
    bool m_seen { false };
    NodeList m_members {};
    QString m_logicalModuleName {};
    QString m_logicalModuleVersionMajor {};
    QString m_logicalModuleVersionMinor {};
    QString m_qtVariable {};
    QString m_qtCMakeComponent {};
};

QT_END_NAMESPACE

#endif // COLLECTIONNODE_H
