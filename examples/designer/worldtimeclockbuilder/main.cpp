// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#include <QtUiTools>
//! [0]
#include <QApplication>
#include <QWidget>
#include <QFile>
//! [1]
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(worldtimeclockbuilder);

    QApplication app(argc, argv);

    QUiLoader loader;
//! [1]

//! [2]
    QFile file(":/forms/form.ui");
    file.open(QFile::ReadOnly);

    QWidget *widget = loader.load(&file);

    file.close();
    widget->show();
//! [2]

//! [3]
    return app.exec();
}
//! [3]
