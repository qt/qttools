// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qpixeltool.h"

#include <qapplication.h>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>
#include <qfileinfo.h>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("PixelTool"_L1);
    QCoreApplication::setApplicationVersion(QLatin1StringView(qVersion()));
    QCoreApplication::setOrganizationName("QtProject"_L1);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("preview"_L1,
                                 "The preview image to show."_L1);

    parser.process(app);

    QPixelTool pixelTool;

    if (!parser.positionalArguments().isEmpty()) {
        const QString previewImageFileName = parser.positionalArguments().constFirst();
        if (QFileInfo::exists(previewImageFileName)) {
            QImage previewImage(previewImageFileName);
            if (!previewImage.size().isEmpty())
                pixelTool.setPreviewImage(previewImage);
        }
    }

    pixelTool.show();

    QObject::connect(&app, &QApplication::lastWindowClosed,
                     &app, &QCoreApplication::quit);

    return QCoreApplication::exec();
}
