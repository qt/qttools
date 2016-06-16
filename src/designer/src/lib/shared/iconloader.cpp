/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#include "iconloader_p.h"

#include <QtCore/QFile>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDESIGNER_SHARED_EXPORT QIcon createIconSet(const QString &name)
{
    QStringList candidates = QStringList()
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/") + name)
#ifdef Q_OS_MAC
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/mac/") + name)
#else
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/win/") + name)
#endif
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/designer_") + name);

    foreach (const QString &f, candidates) {
        if (QFile::exists(f))
            return QIcon(f);
    }

    return QIcon();
}

QDESIGNER_SHARED_EXPORT QIcon emptyIcon()
{
    return QIcon(QStringLiteral(":/qt-project.org/formeditor/images/emptyicon.png"));
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

