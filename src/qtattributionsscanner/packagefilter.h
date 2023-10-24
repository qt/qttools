// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
