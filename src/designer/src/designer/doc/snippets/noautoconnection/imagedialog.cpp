// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QMessageBox>

#include "imagedialog.h"

//! [0]
ImageDialog::ImageDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    okButton->setAutoDefault(false);
    cancelButton->setAutoDefault(false);
//! [0]

    colorDepthCombo->addItem(tr("2 colors (1 bit per pixel)"));
    colorDepthCombo->addItem(tr("4 colors (2 bits per pixel)"));
    colorDepthCombo->addItem(tr("16 colors (4 bits per pixel)"));
    colorDepthCombo->addItem(tr("256 colors (8 bits per pixel)"));
    colorDepthCombo->addItem(tr("65536 colors (16 bits per pixel)"));
    colorDepthCombo->addItem(tr("16 million colors (24 bits per pixel)"));

    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
//! [1]
    connect(okButton, &QAbstractButton::clicked, this, &ImageDialog::checkValues);
}
//! [1]

//! [2]
void ImageDialog::checkValues()
{
    if (nameLineEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("No Image Name"),
            tr("Please supply a name for the image."), QMessageBox::Cancel);
    } else {
        accept();
    }
}
//! [2]
