// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("QtProject"));
    app.setApplicationName(QStringLiteral("Qt Distance Field Generator"));
    app.setApplicationVersion(QStringLiteral(QT_VERSION_STR));

    QCommandLineParser parser;
    parser.setApplicationDescription(
                QCoreApplication::translate("main",
                                            "Allows to prepare a font cache for Qt applications."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QLatin1String("file"),
                                 QCoreApplication::translate("main",
                                                             "Font file (*.ttf, *.otf)"));
    parser.process(app);

    MainWindow mainWindow;
    if (!parser.positionalArguments().isEmpty())
        mainWindow.open(parser.positionalArguments().constFirst());
    mainWindow.show();

    return app.exec();
}
