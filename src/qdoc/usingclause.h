/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
