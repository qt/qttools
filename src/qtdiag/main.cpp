/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtdiag.h"

#include <QtGui/QGuiApplication>
#include <QtCore/QCommandLineParser>

#include <iostream>
#include <string>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("qtdiag"));
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));
    QCoreApplication::setOrganizationName(QStringLiteral("QtProject"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("qt-project.org"));

    QCommandLineParser commandLineParser;
    const QCommandLineOption noGlOption(QStringLiteral("no-gl"), QStringLiteral("Do not output GL information"));
    const QCommandLineOption glExtensionOption(QStringLiteral("gl-extensions"), QStringLiteral("List GL extensions"));
    const QCommandLineOption fontOption(QStringLiteral("fonts"), QStringLiteral("Output list of fonts"));
    commandLineParser.setApplicationDescription(QStringLiteral("Prints diagnostic output about the Qt library."));
    commandLineParser.addOption(noGlOption);
    commandLineParser.addOption(glExtensionOption);
    commandLineParser.addOption(fontOption);
    commandLineParser.addHelpOption();
    commandLineParser.process(app);
    unsigned flags = commandLineParser.isSet(noGlOption) ? 0u : unsigned(QtDiagGl);
    if (commandLineParser.isSet(glExtensionOption))
        flags |= QtDiagGlExtensions;
    if (commandLineParser.isSet(fontOption))
        flags |= QtDiagFonts;

    std::wcout << qtDiag(flags).toStdWString();
    return 0;
}
