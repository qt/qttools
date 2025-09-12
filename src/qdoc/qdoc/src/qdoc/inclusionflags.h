// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INCLUSIONFLAGS_H
#define INCLUSIONFLAGS_H

#include <QtCore/QFlags>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

enum class InclusionFlag : quint32 {
    Private         = 0x0001,
    PrivateFunction = 0x0002,
    PrivateType     = 0x0004,  // Classes, Enums, Typedefs
    PrivateVariable = 0x0008,
    Internal        = 0x0010,
};

Q_DECLARE_FLAGS(InclusionFlags, InclusionFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(InclusionFlags)

QT_END_NAMESPACE

#endif // INCLUSIONFLAGS_H

