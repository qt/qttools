// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CALCULATORFORM_H
#define CALCULATORFORM_H

//! [0]
#include "ui_calculatorform.h"
//! [0]

//! [1]
class CalculatorForm : public QWidget, private Ui::CalculatorForm
{
    Q_OBJECT

public:
    explicit CalculatorForm(QWidget *parent = nullptr);

private slots:
    void on_inputSpinBox1_valueChanged(int value);
    void on_inputSpinBox2_valueChanged(int value);
};
//! [1]

#endif
