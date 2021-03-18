/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdesigner.h"
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qdir.h>

#include <stdlib.h>

QT_USE_NAMESPACE

static bool isOptionSet(int argc, char *argv[], const char *option)
{
    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], option) == 0)
            return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(designer);

    // Attributes to be set before QApplication is constructed.
    if (isOptionSet(argc, argv, "--no-scaling")) {
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    } else {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
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
