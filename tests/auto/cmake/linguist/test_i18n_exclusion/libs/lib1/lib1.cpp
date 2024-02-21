// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "lib1.h"

MyObject1::MyObject1(QObject *parent)
    : QObject(parent)
{
}

QString MyObject1::greeting() const
{
    return tr("Hello from MyObject1!");
}
