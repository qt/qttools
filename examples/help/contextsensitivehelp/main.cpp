// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets/QApplication>

#include "wateringconfigdialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WateringConfigDialog dia;
    return dia.exec();
}
