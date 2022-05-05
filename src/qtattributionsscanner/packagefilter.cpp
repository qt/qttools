/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
