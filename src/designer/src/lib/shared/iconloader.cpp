// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "iconloader_p.h"

#include <QtCore/qfile.h>
#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

QDESIGNER_SHARED_EXPORT QIcon createIconSet(const QString &name)
{
    constexpr QLatin1StringView prefixes[] = {
        ":/qt-project.org/formeditor/images/"_L1,
#ifdef Q_OS_MACOS
        ":/qt-project.org/formeditor/images/mac/"_L1,
#else
        ":/qt-project.org/formeditor/images/win/"_L1,
#endif
        ":/qt-project.org/formeditor/images/designer_"_L1
    };

    for (QLatin1StringView prefix : prefixes) {
        const QString f = prefix + name;
        if (QFile::exists(f))
            return QIcon(f);
    }

    return QIcon();
}

QDESIGNER_SHARED_EXPORT QIcon emptyIcon()
{
    return QIcon(u":/qt-project.org/formeditor/images/emptyicon.png"_s);
}

static QIcon buildIcon(const QString &prefix, const int *sizes, size_t sizeCount)
{
    QIcon result;
    for (size_t i = 0; i < sizeCount; ++i) {
        const QString size = QString::number(sizes[i]);
        const QPixmap pixmap(prefix + size + 'x'_L1 + size + ".png"_L1);
        Q_ASSERT(!pixmap.size().isEmpty());
        result.addPixmap(pixmap);
    }
    return result;
}

QDESIGNER_SHARED_EXPORT QIcon qtLogoIcon()
{
    static const int sizes[] = {16, 24, 32, 64, 128};
    static const QIcon result =
        buildIcon(u":/qt-project.org/formeditor/images/qtlogo"_s,
                  sizes, sizeof(sizes) / sizeof(sizes[0]));
    return result;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

