// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef NODECONTEXT_H
#define NODECONTEXT_H

#include "genustypes.h"
#include "inclusionflags.h"

#include <QtGlobal>

QT_BEGIN_NAMESPACE

struct NodeContext
{
    NodeType type {NodeType::NoType};
    bool isPrivate {false};
    bool isPureVirtual {false};

    InclusionFlags toFlags() const {
        if (!isPrivate)
            return {};

        InclusionFlags flags = InclusionFlag::Private;

        switch (type) {
        case NodeType::Function:
            flags |= InclusionFlag::PrivateFunction;
            break;
        case NodeType::Class:
        case NodeType::Enum:
        case NodeType::Typedef:
            flags |= InclusionFlag::PrivateType;
            break;
        case NodeType::Variable:
            flags |= InclusionFlag::PrivateVariable;
            break;
        default:
            break;
        }

        return flags;
    }
};

QT_END_NAMESPACE

#endif // NODECONTEXT_H

