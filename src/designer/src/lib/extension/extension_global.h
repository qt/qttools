/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef EXTENSION_GLOBAL_H
#define EXTENSION_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#define QDESIGNER_EXTENSION_EXTERN Q_DECL_EXPORT
#define QDESIGNER_EXTENSION_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_EXTENSION_EXPORT
#elif defined(QDESIGNER_EXTENSION_LIBRARY)
#  define QDESIGNER_EXTENSION_EXPORT QDESIGNER_EXTENSION_EXTERN
#else
#  define QDESIGNER_EXTENSION_EXPORT QDESIGNER_EXTENSION_IMPORT
#endif

QT_END_NAMESPACE

#endif // EXTENSION_GLOBAL_H
