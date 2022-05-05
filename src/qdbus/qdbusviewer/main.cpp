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
    while (args.count()) {
        QString arg = args.takeFirst();
        if (arg == QLatin1String("--bus"))
            mw.addCustomBusTab(args.takeFirst());
    }

    mw.show();

    return app.exec();
}
