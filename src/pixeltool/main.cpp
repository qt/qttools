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

#include "qpixeltool.h"

#include <qapplication.h>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>
#include <qfileinfo.h>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("PixelTool"));
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));
    QCoreApplication::setOrganizationName(QLatin1String("QtProject"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QLatin1String("preview"),
                                 QLatin1String("The preview image to show."));

    parser.process(app);

    QPixelTool pixelTool;

    if (!parser.positionalArguments().isEmpty()) {
        const QString previewImageFileName = parser.positionalArguments().constFirst();
        if (QFileInfo(previewImageFileName).exists()) {
            QImage previewImage(previewImageFileName);
            if (!previewImage.size().isEmpty())
                pixelTool.setPreviewImage(previewImage);
        }
    }

    pixelTool.show();

    QObject::connect(&app, &QApplication::lastWindowClosed,
                     &app, &QCoreApplication::quit);

    return app.exec();
}
