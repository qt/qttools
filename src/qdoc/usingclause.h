// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef USINGCLAUSE_H
#define USINGCLAUSE_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

#include <utility>

QT_BEGIN_NAMESPACE

class Node;

struct UsingClause
{
    UsingClause() = default;
    explicit UsingClause(QString signature) : m_signature(std::move(signature)) { }
    [[nodiscard]] const QString &signature() const { return m_signature; }
    [[nodiscard]] const Node *node() const { return m_node; }
    void setNode(const Node *n) { m_node = n; }

    const Node *m_node { nullptr };
    QString m_signature {};
};

QT_END_NAMESPACE

#endif // USINGCLAUSE_H
