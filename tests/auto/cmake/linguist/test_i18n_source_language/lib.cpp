// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include <QCoreApplication>
#include <QDebug>

void printStuff()
{
    qDebug()
        << QObject::tr("We must not see this in the native language's .tr file.")
        << QObject::tr("%n argument(s) passed", "", 156);
}
