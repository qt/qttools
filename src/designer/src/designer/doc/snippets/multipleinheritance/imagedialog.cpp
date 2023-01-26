// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "imagedialog.h"

ImageDialog::ImageDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    colorDepthCombo->addItem(tr("2 colors (1 bit per pixel)"));
    colorDepthCombo->addItem(tr("4 colors (2 bits per pixel)"));
    colorDepthCombo->addItem(tr("16 colors (4 bits per pixel)"));
    colorDepthCombo->addItem(tr("256 colors (8 bits per pixel)"));
    colorDepthCombo->addItem(tr("65536 colors (16 bits per pixel)"));
    colorDepthCombo->addItem(tr("16 million colors (24 bits per pixel)"));

    connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
}
