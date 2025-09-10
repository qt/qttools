// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef DOCUTILITIES_H
#define DOCUTILITIES_H

#include "macro.h"
#include "singleton.h"

#include <QtCore/qglobal.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

typedef QHash<QString, int> QHash_QString_int;
typedef QHash<QString, Macro> QHash_QString_Macro;

struct DocUtilities : public Singleton<DocUtilities>
{
public:
    QHash_QString_int cmdHash;
    QHash_QString_Macro macroHash;
};

QT_END_NAMESPACE

#endif // DOCUTILITIES_H
