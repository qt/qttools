// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>

void excluded1()
{
    QCoreApplication::translate("app1", "excluded1");
}
