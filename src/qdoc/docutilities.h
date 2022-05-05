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
#ifndef DOCUTILITIES_H
#define DOCUTILITIES_H

#include "macro.h"
#include "singleton.h"

#include <QtCore/qglobal.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

typedef QMap<QString, QString> QStringMap;
typedef QHash<QString, int> QHash_QString_int;
typedef QHash<QString, Macro> QHash_QString_Macro;

struct DocUtilities : public Singleton<DocUtilities>
{
public:
    QStringMap aliasMap;
    QHash_QString_int cmdHash;
    QHash_QString_Macro macroHash;
};

QT_END_NAMESPACE

#endif // DOCUTILITIES_H
