// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PACKAGEFILTER_H
#define PACKAGEFILTER_H

#include "package.h"

struct PackageFilter
{
    PackageFilter(const QString &expression);

    bool operator()(const Package &p);

    enum {
        InvalidFilter,
        QDocModuleFilter
    } type;
    QString expression;
};

#endif // PACKAGEFILTER_H
