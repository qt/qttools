// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SIGNALSLOTEDITOR_GLOBAL_H
#define SIGNALSLOTEDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_SIGNALSLOTEDITOR_LIBRARY
# define QT_SIGNALSLOTEDITOR_EXPORT
#else
# define QT_SIGNALSLOTEDITOR_EXPORT
#endif
#else
#define QT_SIGNALSLOTEDITOR_EXPORT
#endif

#endif // SIGNALSLOTEDITOR_GLOBAL_H
