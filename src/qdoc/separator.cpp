/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

/*
  separator.cpp
*/

#include "separator.h"

#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

QString separator(int index, int count)
{
    if (index == count - 1)
        return QCoreApplication::translate("QDoc", ".", "terminator");
    if (count == 2)
        return QCoreApplication::translate("QDoc", " and ", "separator when N = 2");
    if (index == 0)
        return QCoreApplication::translate("QDoc", ", ", "first separator when N > 2");
    if (index < count - 2)
        return QCoreApplication::translate("QDoc", ", ", "general separator when N > 2");
    return QCoreApplication::translate("QDoc", ", and ", "last separator when N > 2");
}

QString comma(int index, int count)
{
    if (index == count - 1)
        return QString();
    if (count == 2)
        return QCoreApplication::translate("QDoc", " and ", "separator when N = 2");
    if (index == 0)
        return QCoreApplication::translate("QDoc", ", ", "first separator when N > 2");
    if (index < count - 2)
        return QCoreApplication::translate("QDoc", ", ", "general separator when N > 2");
    return QCoreApplication::translate("QDoc", ", and ", "last separator when N > 2");
}

QT_END_NAMESPACE
