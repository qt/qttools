// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef STATUS_H
#define STATUS_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

enum class Status : unsigned char {
    Deprecated,
    Preliminary,
    Active,
    Internal,
    DontDocument
};

QT_END_NAMESPACE

#endif // STATUS_H
