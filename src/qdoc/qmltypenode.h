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

#ifndef QMLTYPENODE_H
#define QMLTYPENODE_H

#include "importrec.h"
#include "aggregate.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class ClassNode;
class CollectionNode;

typedef QList<ImportRec> ImportList;

class QmlTypeNode : public Aggregate
{
public:
    QmlTypeNode(Aggregate *parent, const QString &name, NodeType type = QmlType);
    [[nodiscard]] bool isFirstClassAggregate() const override { return true; }
    [[nodiscard]] bool isQtQuickNode() const override
    {
        return (logicalModuleName() == QLatin1String("QtQuick"));
    }
    ClassNode *classNode() override { return m_classNode; }
    void setClassNode(ClassNode *cn) override { m_classNode = cn; }
    [[nodiscard]] bool isAbstract() const override { return m_abstract; }
    [[nodiscard]] bool isWrapper() const override { return m_wrapper; }
    void setAbstract(bool b) override { m_abstract = b; }
    void setWrapper() override { m_wrapper = true; }
    [[nodiscard]] bool isInternal() const override { return (status() == Internal); }
    [[nodiscard]] QString qmlFullBaseName() const override;
    [[nodiscard]] QString obsoleteLink() const override { return m_obsoleteLink; }
    void setObsoleteLink(const QString &t) override { m_obsoleteLink = t; }
    [[nodiscard]] QString logicalModuleName() const override;
    [[nodiscard]] QString logicalModuleVersion() const override;
    [[nodiscard]] QString logicalModuleIdentifier() const override;
    [[nodiscard]] CollectionNode *logicalModule() const override { return m_logicalModule; }
    void setQmlModule(CollectionNode *t) override { m_logicalModule = t; }

    void setImportList(const ImportList &il) { m_importList = il; }
    [[nodiscard]] const QString &qmlBaseName() const { return m_qmlBaseName; }
    void setQmlBaseName(const QString &name) { m_qmlBaseName = name; }
    [[nodiscard]] QmlTypeNode *qmlBaseNode() const override { return m_qmlBaseNode; }
    void resolveInheritance(NodeMap &previousSearches);
    [[nodiscard]] bool cppClassRequired() const { return m_classNodeRequired; }
    static void addInheritedBy(const Node *base, Node *sub);
    static void subclasses(const Node *base, NodeList &subs);
    static void terminate();
    bool inherits(Aggregate *type);

public:
    static QMultiMap<const Node *, Node *> s_inheritedBy;

private:
    bool m_abstract { false };
    bool m_classNodeRequired { false };
    bool m_wrapper { false };
    ClassNode *m_classNode { nullptr };
    QString m_qmlBaseName {};
    QString m_obsoleteLink {};
    CollectionNode *m_logicalModule { nullptr };
    QmlTypeNode *m_qmlBaseNode { nullptr };
    ImportList m_importList {};
};

class QmlBasicTypeNode : public Aggregate
{
public:
    QmlBasicTypeNode(Aggregate *parent, const QString &name, NodeType type = QmlBasicType);
    [[nodiscard]] bool isFirstClassAggregate() const override { return true; }
};

QT_END_NAMESPACE

#endif // QMLTYPENODE_H
