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

#ifndef IMPORTREC_H
#define IMPORTREC_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

struct ImportRec
{
    QString m_moduleName;
    QString m_majorMinorVersion;
    QString m_importAsName;
    QString m_importUri; // subdirectory of module directory

    ImportRec(const QString &name, const QString &version, const QString &importId,
              const QString &importUri)
        : m_moduleName(name),
          m_majorMinorVersion(version),
          m_importAsName(importId),
          m_importUri(importUri)
    {
    }
    QString &name() { return m_moduleName; }
    QString &version() { return m_majorMinorVersion; }
    QString &importId() { return m_importAsName; }
    QString &importUri() { return m_importUri; }
    bool isEmpty() const { return m_moduleName.isEmpty(); }
};

QT_END_NAMESPACE

#endif // IMPORTREC_H
