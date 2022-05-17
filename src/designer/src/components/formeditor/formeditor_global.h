// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMEDITOR_GLOBAL_H
#define FORMEDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_FORMEDITOR_LIBRARY
# define QT_FORMEDITOR_EXPORT
#else
# define QT_FORMEDITOR_EXPORT
#endif
#else
#define QT_FORMEDITOR_EXPORT
#endif

#endif // FORMEDITOR_GLOBAL_H
