// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
