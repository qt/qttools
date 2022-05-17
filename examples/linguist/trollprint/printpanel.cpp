// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "printpanel.h"

//! [0]
PrintPanel::PrintPanel(QWidget *parent)
    : QWidget(parent)
{
/*
    QLabel *label = new QLabel(tr("<b>TROLL PRINT</b>"));
    label->setAlignment(Qt::AlignCenter);
*/
//! [0]

//! [1]
    twoSidedGroupBox = new QGroupBox(tr("2-sided"));
    twoSidedEnabledRadio = new QRadioButton(tr("Enabled"));
    twoSidedDisabledRadio = new QRadioButton(tr("Disabled"));
//! [1] //! [2]
    twoSidedDisabledRadio->setChecked(true);

    colorsGroupBox = new QGroupBox(tr("Colors"));
    colorsEnabledRadio = new QRadioButton(tr("Enabled"));
    colorsDisabledRadio = new QRadioButton(tr("Disabled"));
//! [2]
    colorsDisabledRadio->setChecked(true);

    QHBoxLayout *twoSidedLayout = new QHBoxLayout;
    twoSidedLayout->addWidget(twoSidedEnabledRadio);
    twoSidedLayout->addWidget(twoSidedDisabledRadio);
    twoSidedGroupBox->setLayout(twoSidedLayout);

    QHBoxLayout *colorsLayout = new QHBoxLayout;
    colorsLayout->addWidget(colorsEnabledRadio);
    colorsLayout->addWidget(colorsDisabledRadio);
    colorsGroupBox->setLayout(colorsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
/*
    mainLayout->addWidget(label);
*/
    mainLayout->addWidget(twoSidedGroupBox);
    mainLayout->addWidget(colorsGroupBox);
    setLayout(mainLayout);
}
