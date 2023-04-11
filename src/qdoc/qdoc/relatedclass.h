// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RELATEDCLASS_H
#define RELATEDCLASS_H

#include "access.h"

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <utility>

QT_BEGIN_NAMESPACE

class ClassNode;

struct RelatedClass
{
    RelatedClass() = default;
    // constructor for resolved base class
    RelatedClass(Access access, ClassNode *node) : m_access(access), m_node(node) {}
    // constructor for unresolved base class
    RelatedClass(Access access, QStringList path) : m_access(access), m_path(std::move(path)) { }
    [[nodiscard]] bool isPrivate() const;

    Access m_access {};
    ClassNode *m_node { nullptr };
    QStringList m_path {};
};

QT_END_NAMESPACE

#endif // RELATEDCLASS_H
