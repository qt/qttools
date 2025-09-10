// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtWidgets/qapplication.h>

#include "mainwindow.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(u"QtProject"_s);
    QCoreApplication::setApplicationName(u"QDBusViewer"_s);

    MainWindow mw;
#if !defined(Q_OS_MACOS) && !defined(Q_OS_WIN)
    app.setWindowIcon(QIcon(":/qt-project.org/qdbusviewer/images/qdbusviewer.png"_L1));
#endif
#ifdef Q_OS_MACOS
    mw.setWindowTitle(qApp->translate("QtDBusViewer", "Qt D-Bus Viewer"));
#endif

    QStringList args = app.arguments();
    while (args.size()) {
        QString arg = args.takeFirst();
        if (arg == "--bus"_L1)
            mw.addCustomBusTab(args.takeFirst());
    }

    mw.show();

    return app.exec();
}
