// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    QmlTypeNode(Aggregate *parent, const QString &name, Node::NodeType type);
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
    static void addInheritedBy(const Node *base, Node *sub);
    static void subclasses(const Node *base, NodeList &subs);
    static void terminate();
    bool inherits(Aggregate *type);

public:
    static QMultiMap<const Node *, Node *> s_inheritedBy;

private:
    bool m_abstract { false };
    bool m_wrapper { false };
    ClassNode *m_classNode { nullptr };
    QString m_qmlBaseName {};
    CollectionNode *m_logicalModule { nullptr };
    QmlTypeNode *m_qmlBaseNode { nullptr };
    ImportList m_importList {};
};

QT_END_NAMESPACE

#endif // QMLTYPENODE_H
