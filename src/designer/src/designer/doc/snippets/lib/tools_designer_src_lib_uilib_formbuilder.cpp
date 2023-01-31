// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        MyForm::MyForm(QWidget *parent)
            : QWidget(parent)
        {
            QFormBuilder builder;
            QFile file(":/forms/myWidget.ui");
            file.open(QFile::ReadOnly);
            QWidget *myWidget = builder.load(&file, this);
            file.close();

            auto *layout = new QVBoxLayout(this);
            layout->addWidget(myWidget);
        }
//! [0]


//! [1]
    <!DOCTYPE RCC><RCC version="1.0">
    <qresource prefix="/forms">
       <file>mywidget.ui</file>
    </qresource>
    </RCC>
//! [1]


