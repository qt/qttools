// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "arrowpad.h"

ArrowPad::ArrowPad(QWidget *parent)
    : QWidget(parent)
{
//! [0]
    upButton = new QPushButton(tr("&Up"));
//! [0] //! [1]
    downButton = new QPushButton(tr("&Down"));
//! [1] //! [2]
    leftButton = new QPushButton(tr("&Left"));
//! [2] //! [3]
    rightButton = new QPushButton(tr("&Right"));
//! [3]

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(upButton, 0, 1);
    mainLayout->addWidget(leftButton, 1, 0);
    mainLayout->addWidget(rightButton, 1, 2);
    mainLayout->addWidget(downButton, 2, 1);
    setLayout(mainLayout);
}
