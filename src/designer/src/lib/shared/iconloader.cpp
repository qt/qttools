// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "iconloader_p.h"

#include <QtCore/qfile.h>
#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDESIGNER_SHARED_EXPORT QIcon createIconSet(const QString &name)
{
    const QStringList candidates = QStringList()
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/") + name)
#ifdef Q_OS_MACOS
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/mac/") + name)
#else
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/win/") + name)
#endif
        << (QString::fromUtf8(":/qt-project.org/formeditor/images/designer_") + name);

    for (const QString &f : candidates) {
        if (QFile::exists(f))
            return QIcon(f);
    }

    return QIcon();
}

QDESIGNER_SHARED_EXPORT QIcon emptyIcon()
{
    return QIcon(QStringLiteral(":/qt-project.org/formeditor/images/emptyicon.png"));
}

static QIcon buildIcon(const QString &prefix, const int *sizes, size_t sizeCount)
{
    QIcon result;
    for (size_t i = 0; i < sizeCount; ++i) {
        const QString size = QString::number(sizes[i]);
        const QPixmap pixmap(prefix + size + QLatin1Char('x') + size + QStringLiteral(".png"));
        Q_ASSERT(!pixmap.size().isEmpty());
        result.addPixmap(pixmap);
    }
    return result;
}

QDESIGNER_SHARED_EXPORT QIcon qtLogoIcon()
{
    static const int sizes[] = {16, 24, 32, 64};
    static const QIcon result =
        buildIcon(QStringLiteral(":/qt-project.org/formeditor/images/qtlogo"),
                  sizes, sizeof(sizes) / sizeof(sizes[0]));
    return result;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

