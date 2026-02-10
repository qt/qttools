// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_BUILDER_H
#define QDOC_IR_BUILDER_H

#include "document.h"

QT_BEGIN_NAMESPACE

class PageNode;

namespace IR {

class Builder
{
public:
    Builder() = default;

    [[nodiscard]] Document buildPageIR(const PageNode *pn) const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_BUILDER_H

