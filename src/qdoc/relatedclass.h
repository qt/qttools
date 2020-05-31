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

#ifndef RELATEDCLASS_H
#define RELATEDCLASS_H

#include "access.h"

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class ClassNode;

struct RelatedClass
{
    RelatedClass() = default;
    // constructor for resolved base class
    RelatedClass(Access access, ClassNode *node) : m_access(access), m_node(node) {}
    // constructor for unresolved base class
    RelatedClass(Access access, const QStringList &path, const QString &signature)
        : m_access(access), m_path(path), m_signature(signature)
    {
    }
    QString accessString() const;
    bool isPrivate() const;

    Access m_access {};
    ClassNode *m_node { nullptr };
    QStringList m_path {};
    QString m_signature {};
};

QT_END_NAMESPACE

#endif // RELATEDCLASS_H
