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
