// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TABORDEREDITOR_GLOBAL_H
#define TABORDEREDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_TABORDEREDITOR_LIBRARY
# define QT_TABORDEREDITOR_EXPORT
#else
# define QT_TABORDEREDITOR_EXPORT
#endif
#else
#define QT_TABORDEREDITOR_EXPORT
#endif

#endif // TABORDEREDITOR_GLOBAL_H
