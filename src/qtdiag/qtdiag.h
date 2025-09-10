// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
