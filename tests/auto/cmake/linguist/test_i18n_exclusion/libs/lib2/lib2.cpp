// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include "lib2.h"

MyObject2::MyObject2(QObject *parent)
    : QObject(parent)
{
}

QString MyObject2::greeting() const
{
    return tr("Hello from MyObject2!");
}
