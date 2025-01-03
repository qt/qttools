// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef VIEW3D_GLOBAL_H
#define VIEW3D_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifdef Q_OS_WIN
#ifdef VIEW3D_LIBRARY
# define VIEW3D_EXPORT
#else
# define VIEW3D_EXPORT
#endif
#else
#define VIEW3D_EXPORT
#endif

QT_END_NAMESPACE

#endif // VIEW3D_GLOBAL_H
