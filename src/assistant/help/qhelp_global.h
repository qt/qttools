// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELP_GLOBAL_H
#define QHELP_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE


#ifdef QT_STATIC
#   define QHELP_EXPORT
#elif defined(QHELP_LIB)
#   define QHELP_EXPORT Q_DECL_EXPORT
#else
#   define QHELP_EXPORT Q_DECL_IMPORT
#endif

class QHELP_EXPORT QHelpGlobal {
public:
    static QString uniquifyConnectionName(const QString &name, void *pointer);
    static QString documentTitle(const QString &content);
};

QT_END_NAMESPACE

#endif // QHELP_GLOBAL_H
