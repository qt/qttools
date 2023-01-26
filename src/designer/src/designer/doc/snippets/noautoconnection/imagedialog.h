// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include "ui_imagedialog.h"

//! [0]
class ImageDialog : public QDialog, private Ui::ImageDialog
{
    Q_OBJECT

public:
    explicit ImageDialog(QWidget *parent = nullptr);

private slots:
    void checkValues();
};
//! [0]

#endif
