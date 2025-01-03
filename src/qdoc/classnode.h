// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CLASSNODE_H
#define CLASSNODE_H

#include "aggregate.h"
#include "relatedclass.h"
#include "usingclause.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class FunctionNode;
class PropertyNode;
class QmlTypeNode;

class ClassNode : public Aggregate
{
public:
    ClassNode(NodeType type, Aggregate *parent, const QString &name) : Aggregate(type, parent, name)
    {
    }
    [[nodiscard]] bool isFirstClassAggregate() const override { return true; }
    [[nodiscard]] bool isClassNode() const override { return true; }
    [[nodiscard]] bool isRelatableType() const override { return true; }
    [[nodiscard]] bool isWrapper() const override { return m_wrapper; }
    [[nodiscard]] QString obsoleteLink() const override { return m_obsoleteLink; }
    void setObsoleteLink(const QString &t) override { m_obsoleteLink = t; }
    void setWrapper() override { m_wrapper = true; }

    void addResolvedBaseClass(Access access, ClassNode *node);
    void addDerivedClass(Access access, ClassNode *node);
    void addUnresolvedBaseClass(Access access, const QStringList &path);
    void removePrivateAndInternalBases();
    void resolvePropertyOverriddenFromPtrs(PropertyNode *pn);

    QList<RelatedClass> &baseClasses() { return m_bases; }
    QList<RelatedClass> &derivedClasses() { return m_derived; }
    QList<RelatedClass> &ignoredBaseClasses() { return m_ignoredBases; }
    QList<UsingClause> &usingClauses() { return m_usingClauses; }

    [[nodiscard]] const QList<RelatedClass> &baseClasses() const { return m_bases; }

    QmlTypeNode *qmlElement() { return m_qmlElement; }
    void setQmlElement(QmlTypeNode *qcn) { m_qmlElement = qcn; }
    [[nodiscard]] bool isAbstract() const override { return m_abstract; }
    void setAbstract(bool b) override { m_abstract = b; }
    PropertyNode *findPropertyNode(const QString &name);
    QmlTypeNode *findQmlBaseNode();
    FunctionNode *findOverriddenFunction(const FunctionNode *fn);
    PropertyNode *findOverriddenProperty(const FunctionNode *fn);
    [[nodiscard]] bool docMustBeGenerated() const override;

private:
    void promotePublicBases(const QList<RelatedClass> &bases);

private:
    QList<RelatedClass> m_bases {};
    QList<RelatedClass> m_derived {};
    QList<RelatedClass> m_ignoredBases {};
    QList<UsingClause> m_usingClauses {};
    bool m_abstract { false };
    bool m_wrapper { false };
    QString m_obsoleteLink {};
    QmlTypeNode *m_qmlElement { nullptr };
};

QT_END_NAMESPACE

#endif // CLASSNODE_H
