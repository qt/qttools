// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CLASSNODE_H
#define CLASSNODE_H

#include "aggregate.h"
#include "relatedclass.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class FunctionNode;
class PropertyNode;
class QmlTypeNode;
class Tree;

class ClassNode : public Aggregate
{
    friend class Tree;

public:
    ClassNode(NodeType type, Aggregate *parent, const QString &name) : Aggregate(type, parent, name)
    {
    }
    [[nodiscard]] bool isFirstClassAggregate() const override { return true; }
    [[nodiscard]] bool isClassNode() const override { return true; }
    [[nodiscard]] bool isRelatableType() const override { return true; }
    [[nodiscard]] bool isWrapper() const override { return m_wrapper; }
    void setWrapper() override { m_wrapper = true; }

    void addResolvedBaseClass(Access access, ClassNode *node);
    void addDerivedClass(Access access, ClassNode *node);
    void addUnresolvedBaseClass(Access access, const QStringList &path);
    void removePrivateAndInternalBases();
    void resolvePropertyOverriddenFromPtrs(PropertyNode *pn);

    [[nodiscard]] const QList<RelatedClass> &baseClasses() const { return m_bases; }
    [[nodiscard]] const QList<RelatedClass> &derivedClasses() const { return m_derived; }
    [[nodiscard]] const QList<RelatedClass> &ignoredBaseClasses() const { return m_ignoredBases; }

    [[nodiscard]] bool isAbstract() const override { return m_abstract; }
    void setAbstract(bool b) override { m_abstract = b; }
    PropertyNode *findPropertyNode(const QString &name);
    FunctionNode *findOverriddenFunction(const FunctionNode *fn);
    PropertyNode *findOverriddenProperty(const FunctionNode *fn);
    [[nodiscard]] bool docMustBeGenerated() const override;

    void insertQmlNativeType(QmlTypeNode *qmlTypeNode) { m_nativeTypeForQml << qmlTypeNode; }
    bool isQmlNativeType() { return !m_nativeTypeForQml.empty(); }
    const QSet<QmlTypeNode *> &qmlNativeTypes() { return m_nativeTypeForQml; }

    enum class HierarchyDirection { Base, Derived };

    [[nodiscard]] bool hasCircularInheritance(QStringList *cyclePath = nullptr) const;
    [[nodiscard]] bool hasCircularDerivedClasses(QStringList *cyclePath = nullptr) const;

private:
    enum class Color { White, Gray, Black };

    [[nodiscard]] bool hasCircularRelationship(HierarchyDirection direction,
                                               QStringList *cyclePath) const;
    [[nodiscard]] bool detectCycleRecursive(HierarchyDirection direction,
                                            QMap<const ClassNode*, Color> &colors,
                                            QList<const ClassNode*> &path) const;

private:
    void promotePublicBases(const QList<RelatedClass> &bases);

    QList<RelatedClass> &baseClasses_mutable() { return m_bases; }

private:
    QList<RelatedClass> m_bases {};
    QList<RelatedClass> m_derived {};
    QList<RelatedClass> m_ignoredBases {};
    bool m_abstract { false };
    bool m_wrapper { false };
    QSet<QmlTypeNode *> m_nativeTypeForQml;
};

QT_END_NAMESPACE

#endif // CLASSNODE_H
