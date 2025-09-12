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
    [[nodiscard]] static bool isIncluded(const InclusionPolicy& policy,
                                     const NodeContext& context) {
        if (context.isInternal && !policy.showInternal)
            return false;

        if (!context.isPrivate)
            return true;

        if (context.isPureVirtual)
            return true;

        InclusionFlags policyFlags = policy.toFlags();
        InclusionFlags nodeFlags = context.toFlags();

        return (policyFlags & nodeFlags) != 0;
    }

    [[nodiscard]] static bool requiresDocumentation(const InclusionPolicy& policy,
                                           const NodeContext& context) {
        if (context.isInternal && !policy.showInternal)
            return false;

        if (!context.isPrivate)
            return true;  // Always warn about non-private undocumented

        InclusionFlags policyFlags = policy.toFlags();
        InclusionFlags nodeFlags = context.toFlags();

        return (policyFlags & nodeFlags) != 0;
    }

    [[nodiscard]] static bool isPubliclyVisible(const InclusionPolicy& policy,
                                              const NodeContext& context) {
        return !context.isInternal || policy.showInternal;
    }

    [[nodiscard]] static bool processInternalDocs(const InclusionPolicy& policy) {
        return policy.showInternal;
    }

    [[nodiscard]] static bool isReimplementedMemberVisible(const InclusionPolicy& policy,
                                                         const NodeContext& context) {
        // Business rule: "Reimplemented members are visible based on their access level
        // in the derived class, regardless of internal status"

        if (!context.isPrivate)
            return true;  // Public and protected always visible

        if (context.isPureVirtual)
            return true;  // Pure virtual always visible

        // For private members, check inclusion policy but ignore internal status
        // We create a non-internal context to bypass internal checks in isIncluded()
        NodeContext nonInternalContext = context;
        nonInternalContext.isInternal = false;

        return isIncluded(policy, nonInternalContext);
    }
};

QT_END_NAMESPACE

#endif // INCLUSIONFILTER_H

