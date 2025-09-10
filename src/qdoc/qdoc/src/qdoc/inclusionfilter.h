// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INCLUSIONFILTER_H
#define INCLUSIONFILTER_H

#include "inclusionpolicy.h"
#include "nodecontext.h"

#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

class InclusionFilter {
public:
    static bool shouldIncludePrivate(const InclusionPolicy& policy,
                                     const NodeContext& context) {
        if (!context.isPrivate)
            return true;

        if (context.isPureVirtual)
            return true;

        InclusionFlags policyFlags = policy.toFlags();
        InclusionFlags nodeFlags = context.toFlags();

        return (policyFlags & nodeFlags) != 0;
    }

    static bool shouldWarnAboutUndocumented(const InclusionPolicy& policy,
                                           const NodeContext& context) {
        if (!context.isPrivate)
            return true;  // Always warn about non-private undocumented

        InclusionFlags policyFlags = policy.toFlags();
        InclusionFlags nodeFlags = context.toFlags();

        return (policyFlags & nodeFlags) != 0;
    }
};

QT_END_NAMESPACE

#endif // INCLUSIONFILTER_H

