// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROPERTYEDITOR_GLOBAL_H
#define PROPERTYEDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifdef Q_OS_WIN
#ifdef QT_PROPERTYEDITOR_LIBRARY
# define QT_PROPERTYEDITOR_EXPORT
#else
# define QT_PROPERTYEDITOR_EXPORT
#endif
#else
#define QT_PROPERTYEDITOR_EXPORT
#endif

QT_END_NAMESPACE

#endif // PROPERTYEDITOR_GLOBAL_H
