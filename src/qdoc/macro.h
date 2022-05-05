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
