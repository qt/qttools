/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#include "qdesigner.h"
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qoperatingsystemversion.h>

#include <stdlib.h>

QT_USE_NAMESPACE

static const char rhiBackEndVar[] = "QSG_RHI_BACKEND";

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(designer);

    // Enable the QWebEngineView, QQuickWidget plugins on Windows.
    if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows
        && !qEnvironmentVariableIsSet(rhiBackEndVar)) {
        qputenv(rhiBackEndVar, "gl");
    }

    // required for QWebEngineView
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QDesigner app(argc, argv);
    switch (app.parseCommandLineArguments()) {
    case QDesigner::ParseArgumentsSuccess:
        break;
    case QDesigner::ParseArgumentsError:
        return 1;
    case QDesigner::ParseArgumentsHelpRequested:
        return 0;
    }
    QGuiApplication::setQuitOnLastWindowClosed(false);

    return QApplication::exec();
}
