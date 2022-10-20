/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

    void setOutputFileName(const QString &f) override { m_outputFileName = f; }
    [[nodiscard]] QString outputFileName() const override { return m_outputFileName; }

protected:
    friend class Node;

protected:
    bool m_noAutoList { false };
    QString m_title {};
    QString m_subtitle {};
    QString m_outputFileName {};
    QStringList m_groupNames {};
};

QT_END_NAMESPACE

#endif // PAGENODE_H
