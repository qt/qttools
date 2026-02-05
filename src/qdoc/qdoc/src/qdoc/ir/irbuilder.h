// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef IRBUILDER_H
#define IRBUILDER_H

#include "documentir.h"

QT_BEGIN_NAMESPACE

class PageNode;

class IRBuilder
{
public:
    IRBuilder() = default;

    [[nodiscard]] DocumentIR buildPageIR(const PageNode *pn) const;
};

QT_END_NAMESPACE

#endif // IRBUILDER_H

