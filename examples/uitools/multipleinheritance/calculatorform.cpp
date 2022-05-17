// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "calculatorform.h"
#include <QWidget>


//! [0]
CalculatorForm::CalculatorForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}
//! [0]

//! [1]
void CalculatorForm::on_inputSpinBox1_valueChanged(int value)
{
    outputWidget->setText(QString::number(value + inputSpinBox2->value()));
}
//! [1]

//! [2]
void CalculatorForm::on_inputSpinBox2_valueChanged(int value)
{
    outputWidget->setText(QString::number(value + inputSpinBox1->value()));
}
//! [2]
