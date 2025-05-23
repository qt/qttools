// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef TASKMENU_GLOBAL_H
#define TASKMENU_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_TASKMENU_LIBRARY
# define QT_TASKMENU_EXPORT
#else
# define QT_TASKMENU_EXPORT
#endif
#else
#define QT_TASKMENU_EXPORT
#endif

#endif // TASKMENU_GLOBAL_H
