/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
