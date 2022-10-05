// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtWidgets/qapplication.h>

#include "mainwindow.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("QtProject"));
    QCoreApplication::setApplicationName(QStringLiteral("QDBusViewer"));

    MainWindow mw;
#if !defined(Q_OS_OSX) && !defined(Q_OS_WIN)
    app.setWindowIcon(QIcon(QLatin1String(":/qt-project.org/qdbusviewer/images/qdbusviewer.png")));
#endif
#ifdef Q_OS_OSX
    mw.setWindowTitle(qApp->translate("QtDBusViewer", "Qt D-Bus Viewer"));
#endif

    QStringList args = app.arguments();
    while (args.size()) {
        QString arg = args.takeFirst();
        if (arg == QLatin1String("--bus"))
            mw.addCustomBusTab(args.takeFirst());
    }

    mw.show();

    return app.exec();
}
