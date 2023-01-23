// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CALCULATORFORM_H
#define CALCULATORFORM_H

//! [0]
#include "ui_calculatorform.h"
//! [0]

//! [1]
class CalculatorForm : public QWidget
{
    Q_OBJECT

public:
    explicit CalculatorForm(QWidget *parent = nullptr);

private slots:
    void updateResult();

private:
    Ui::CalculatorForm ui;
};
//! [1]

#endif
