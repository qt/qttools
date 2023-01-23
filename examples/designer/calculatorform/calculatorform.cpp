// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "calculatorform.h"

//! [0]
CalculatorForm::CalculatorForm(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.inputSpinBox1, &QSpinBox::valueChanged, this, &CalculatorForm::updateResult);
    connect(ui.inputSpinBox2, &QSpinBox::valueChanged, this, &CalculatorForm::updateResult);
}
//! [0]

//! [1]
void CalculatorForm::updateResult()
{
    const int sum = ui.inputSpinBox1->value() + ui.inputSpinBox2->value();
    ui.outputWidget->setText(QString::number(sum));
}
//! [1]
