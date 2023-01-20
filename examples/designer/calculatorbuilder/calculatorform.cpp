// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "calculatorform.h"

//! [0]
#include <QtUiTools>
//! [0]
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include <QFile>

using namespace Qt::StringLiterals;

//! [1]
CalculatorForm::CalculatorForm(QWidget *parent)
    : QWidget(parent)
{
    QUiLoader loader;

    QFile file(u":/forms/calculatorform.ui"_s);
    file.open(QFile::ReadOnly);
    QWidget *formWidget = loader.load(&file, this);
    file.close();
//! [1]

//! [2]
    ui_inputSpinBox1 = findChild<QSpinBox*>(u"inputSpinBox1"_s);
    ui_inputSpinBox2 = findChild<QSpinBox*>(u"inputSpinBox2"_s);
    ui_outputWidget = findChild<QLabel*>(u"outputWidget"_s);
//! [2]

//! [3]
    QMetaObject::connectSlotsByName(this);
//! [3]

//! [4]
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(formWidget);

    setWindowTitle(tr("Calculator Builder"));
}
//! [4]

//! [5]
void CalculatorForm::on_inputSpinBox1_valueChanged(int value)
{
    ui_outputWidget->setText(QString::number(value + ui_inputSpinBox2->value()));
}
//! [5] //! [6]

//! [6] //! [7]
void CalculatorForm::on_inputSpinBox2_valueChanged(int value)
{
    ui_outputWidget->setText(QString::number(value + ui_inputSpinBox1->value()));
}
//! [7]
