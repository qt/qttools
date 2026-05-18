// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef OBJECTUTILS_P_H
#define OBJECTUTILS_P_H

#if 0
#  pragma qt_sync_skip_header_check
#endif

#include "uilib_global.h"

#include <QtCore/qcontainerfwd.h>

QT_BEGIN_NAMESPACE

class QLayout;
class QWidget;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

QDESIGNER_UILIB_EXPORT QStringList layoutNames();
QDESIGNER_UILIB_EXPORT QStringList widgetNames();

QDESIGNER_UILIB_EXPORT QLayout *createLayoutInstance(const QString &className, QWidget *parent);
QDESIGNER_UILIB_EXPORT QWidget *createWidgetInstance(const QString &className, QWidget *parent);

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // OBJECTUTILS_P_H
