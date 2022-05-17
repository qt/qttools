// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "mainwindow.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto locale = QLocale::system();

//! [0]
    QTranslator translator;
    if (translator.load(locale, u"trollprint"_s, u"_"_s))
        app.installTranslator(&translator);
//! [0]

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
