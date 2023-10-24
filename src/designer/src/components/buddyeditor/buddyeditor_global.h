// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef BUDDYEDITOR_GLOBAL_H
#define BUDDYEDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_BUDDYEDITOR_LIBRARY
# define QT_BUDDYEDITOR_EXPORT
#else
# define QT_BUDDYEDITOR_EXPORT
#endif
#else
#define QT_BUDDYEDITOR_EXPORT
#endif

#endif // BUDDYEDITOR_GLOBAL_H
