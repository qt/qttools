// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PARSERERROR_H
#define PARSERERROR_H

#include "location.h"

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

struct FnMatchError {
    QString signature {};
    Location location {};

};

struct ParserErrorHandler
{
    void operator()(const FnMatchError &e) const;
};

QT_END_NAMESPACE

#endif
