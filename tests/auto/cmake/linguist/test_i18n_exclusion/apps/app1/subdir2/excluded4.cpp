// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#include <QCoreApplication>

class WossName
{
    Q_DECLARE_TR_FUNCTIONS(WossName)
public:
    static QString greeting()
    {
        return tr("excluded4");
    }
};
