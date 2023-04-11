// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef MACRO_H
#define MACRO_H

#include "location.h"

#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

/*!
 * Simple structure used by the Doc and DocParser classes.
 */
struct Macro
{
public:
    QString m_defaultDef {};
    Location m_defaultDefLocation {};
    QMap<QString, QString> m_otherDefs {};
    int numParams {};
};

QT_END_NAMESPACE

#endif // MACRO_H
