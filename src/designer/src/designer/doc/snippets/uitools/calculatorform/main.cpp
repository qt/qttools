// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#include "ui_calculatorform.h"
//! [0]
#include <QApplication>

//! [1]
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget widget;
    Ui::CalculatorForm ui;
    ui.setupUi(&widget);

    widget.show();
    return app.exec();
}
//! [1]
