// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PAGENODE_H
#define PAGENODE_H

#include "node.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class Aggregate;

class PageNode : public Node
{
public:
    PageNode(Aggregate *parent, const QString &name) : Node(Page, parent, name) {}
    PageNode(NodeType type, Aggregate *parent, const QString &name) : Node(type, parent, name) {}
    PageNode(Aggregate *parent, const QString &name, PageType ptype) : Node(Page, parent, name)
    {
        setPageType(ptype);
    }

    [[nodiscard]] bool isPageNode() const override { return true; }
    [[nodiscard]] bool isTextPageNode() const override
    {
        return !isAggregate();
    } // PageNode but not Aggregate

    [[nodiscard]] QString title() const override { return m_title; }
    [[nodiscard]] QString subtitle() const override { return m_subtitle; }
    [[nodiscard]] QString fullTitle() const override;
    bool setTitle(const QString &title) override;
    bool setSubtitle(const QString &subtitle) override
    {
        m_subtitle = subtitle;
        return true;
    }
    [[nodiscard]] QString nameForLists() const override { return title(); }

    [[nodiscard]] virtual QString imageFileName() const { return QString(); }
    virtual void setImageFileName(const QString &) {}

    [[nodiscard]] bool noAutoList() const { return m_noAutoList; }
    void setNoAutoList(bool b) override { m_noAutoList = b; }
    [[nodiscard]] const QStringList &groupNames() const { return m_groupNames; }
    void appendGroupName(const QString &t) override { m_groupNames.append(t); }

    [[nodiscard]] const PageNode *navigationParent() const { return m_navParent; }
    void setNavigationParent(const PageNode *parent) { m_navParent = parent; }

protected:
    friend class Node;

protected:
    bool m_noAutoList { false };
    QString m_title {};
    QString m_subtitle {};
    QStringList m_groupNames {};

private:
    const PageNode *m_navParent { nullptr };
};

QT_END_NAMESPACE

#endif // PAGENODE_H
