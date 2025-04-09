// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QtUiTools>

#include "mywidget.h"

//! [0]
MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
{
    QFile file(":/forms/myform.ui");
    if (!file.open(QFile::ReadOnly))
        qFatal("Cannot open resource file");

    QUiLoader loader;
    QWidget *myWidget = loader.load(&file, this);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(myWidget);
    setLayout(layout);
}
//! [0]
