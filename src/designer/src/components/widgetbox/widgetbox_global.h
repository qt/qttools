// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef WIDGETBOX_GLOBAL_H
#define WIDGETBOX_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_WIDGETBOX_LIBRARY
# define QT_WIDGETBOX_EXPORT
#else
# define QT_WIDGETBOX_EXPORT
#endif
#else
#define QT_WIDGETBOX_EXPORT
#endif

#endif // WIDGETBOX_GLOBAL_H
