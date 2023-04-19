// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QPushButton>
//! [0]
#include <QTranslator>
//! [0]

//! [1] //! [2]
int main(int argc, char *argv[])
//! [1] //! [3] //! [4]
{
    QApplication app(argc, argv);
//! [3]

//! [5]
    QTranslator translator;
//! [5] //! [6]
    Q_UNUSED(translator.load("hellotr_la"));
//! [6] //! [7]
    app.installTranslator(&translator);
//! [4] //! [7]

//! [8]
    QPushButton hello(QPushButton::tr("Hello world!"));
//! [8]
    hello.resize(100, 30);

    hello.show();
    return app.exec();
}
//! [2]
