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

#ifndef QDESIGNER_COMPONENTS_GLOBAL_H
#define QDESIGNER_COMPONENTS_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#define QDESIGNER_COMPONENTS_EXTERN Q_DECL_EXPORT
#define QDESIGNER_COMPONENTS_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_COMPONENTS_EXPORT
#elif defined(QDESIGNER_COMPONENTS_LIBRARY)
#  define QDESIGNER_COMPONENTS_EXPORT QDESIGNER_COMPONENTS_EXTERN
#else
#  define QDESIGNER_COMPONENTS_EXPORT QDESIGNER_COMPONENTS_IMPORT
#endif


QT_END_NAMESPACE
#endif // QDESIGNER_COMPONENTS_GLOBAL_H
