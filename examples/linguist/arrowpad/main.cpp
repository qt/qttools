// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "mainwindow.h"

using namespace Qt::StringLiterals;

//! [0]
int main(int argc, char *argv[])
//! [0] //! [1]
{
    QApplication app(argc, argv);

    auto locale = QLocale::system();

//! [2]
    QTranslator translator;
//! [2] //! [3]
    if (translator.load(locale, u"arrowpad"_s, u"_"_s))
        app.installTranslator(&translator);
//! [1] //! [3]

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
