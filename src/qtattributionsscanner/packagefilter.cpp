// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "logging.h"
#include "packagefilter.h"
#include <iostream>

PackageFilter::PackageFilter(const QString &expression)
    : type(InvalidFilter)
{
    const QLatin1String filter("QDocModule=");
    if (expression.startsWith(filter)) {
        type = QDocModuleFilter;
        this->expression = expression.mid(filter.size());
    } else {
        std::cerr << qPrintable(tr("Invalid filter expression \"%1\"").arg(expression)) << std::endl;
        std::cerr << qPrintable(tr("Currently only \"QDocModule=*\" is supported.")) << std::endl;
    }
}

bool PackageFilter::operator()(const Package &p)
{
    switch (type) {
    case InvalidFilter:
        return true;
    case QDocModuleFilter:
        return p.qdocModule == expression;
    }
    return false;
}
