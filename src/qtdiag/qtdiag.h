/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef QTDIAG_H
#define QTDIAG_H

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

enum QtDiagFlags {
    QtDiagGl = 0x1,
    QtDiagGlExtensions = 0x2,
    QtDiagFonts = 0x4,
    QtDiagVk = 0x8,
    QtDiagRhi = 0x10
};

QString qtDiag(unsigned flags = 0);

QT_END_NAMESPACE

#endif // QTDIAG_H
