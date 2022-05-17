// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include "ui_imagedialog.h"

class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    ImageDialog(QWidget *parent = 0);

private:
    Ui::ImageDialog ui;
};

#endif
