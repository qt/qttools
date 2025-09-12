// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INCLUSIONPOLICY_H
#define INCLUSIONPOLICY_H

#include "inclusionflags.h"

#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

struct InclusionPolicy
{
    bool includePrivate = false;
    bool includePrivateFunction = false;
    bool includePrivateType = false;
    bool includePrivateVariable = false;
    bool showInternal = false;

    [[nodiscard]] InclusionFlags toFlags() const {
        InclusionFlags flags {};
        if (includePrivate)
            flags |= InclusionFlag::Private;
        if (includePrivateFunction)
            flags |= InclusionFlag::PrivateFunction;
        if (includePrivateType)
            flags |= InclusionFlag::PrivateType;
        if (includePrivateVariable)
            flags |= InclusionFlag::PrivateVariable;
        if (showInternal)
            flags |= InclusionFlag::Internal;
        return flags;
    }
};

QT_END_NAMESPACE

#endif // INCLUSIONPOLICY_H

