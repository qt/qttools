// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    void getMemberNamespaces(NodeMap &out);
    void getMemberClasses(NodeMap &out) const;
    [[nodiscard]] bool wasSeen() const override { return m_seen; }

    [[nodiscard]] QString fullTitle() const override { return title(); }
    [[nodiscard]] QString logicalModuleName() const override { return m_logicalModuleName; }
    [[nodiscard]] QString logicalModuleVersion() const override;
    [[nodiscard]] QString logicalModuleIdentifier() const override
    {
        return m_logicalModuleName + m_logicalModuleVersionMajor;
    }
    [[nodiscard]] QString state() const { return m_state; }

    void setLogicalModuleInfo(const QStringList &info) override;
    void setState(const QString &state) { m_state = state; }

    // REMARK: Those methods are used by QDocDatabase as a performance
    // detail to avoid merging a collection node multiple times. They
    // should not be addressed in any other part of the code nor
    // should their usage appear more than once in QDocDatabase,
    // albeit this is not enforced.
    // More information are provided in the comment for the definition
    // of m_merged.
    void markMerged() { m_merged = true; }
    bool isMerged() { return m_merged; }

    [[nodiscard]] const NodeList &members() const { return m_members; }

    void markSeen() { m_seen = true; }
    void markNotSeen() { m_seen = false; }

private:
    bool m_seen { false };
    // REMARK: This is set by the database when merging the collection
    // node and is later used to avoid merging the same collection
    // multiple times.
    // Currently, collection nodes may come from multiple projects,
    // such that to have a complete overview of the members of a
    // collection we need to rejoin all members for all instances of
    // the "same" collection.
    // This is done in QDocDatabase, generally through an external
    // method call that is done ad-hoc when a source-of-truth
    // collection node is needed.
    // As each part of the code that need such a source-of-truth will
    // need to merge the node, to avoid the overhead of a relatively
    // expensive operation being performed multiple times, we expose
    // this detail so that QDocDatabase can avoid performing the
    // operation again.
    // To avoid the coupling, QDocDatabase could keep track of the
    // merged nodes itself, this is a bit less trivial that this
    // implementation and doesn't address the source of the problem
    // (the multiple merges themselves and the sequencing of the
    // related operations) and as such was discarded in favor of this
    // simpler implementation.
    // Do note that outside the very specific purpose for which this
    // member was made, no part of the code should refer to it and its
    // associated methods.
    // Should this start to be the case, we can switch to the more
    // complex encapsulation into QDocDatabase without having to touch
    // the outside user of the merges.
    // Further down the line, this is expected to go away completely
    // as other part of the code are streamlined.
    // KLUDGE: Note that this whole exposure is done as a hackish
    // solution to QTBUG-104237 and should not be considered final or
    // dependable.
    bool m_merged { false };
    NodeList m_members {};
    QString m_logicalModuleName {};
    QString m_logicalModuleVersionMajor {};
    QString m_logicalModuleVersionMinor {};
    QString m_qtVariable {};
    QString m_qtCMakeComponent {};
    QString m_state {};
};

QT_END_NAMESPACE

#endif // COLLECTIONNODE_H
