/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QCommandLineParser>
#include <QtCore/QFile>

#include <iostream>

#include "d3dservice.h"

static QFile outputFile;
static void outputFileMessageHandler(QtMsgType, const QMessageLogContext &, const QString &text)
{
    outputFile.write(text.toUtf8());
    outputFile.write("\r\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Qt D3D Shader Compilation Service"));
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));
    app.setOrganizationName(QStringLiteral("Qt Project"));
    app.setOrganizationDomain(QStringLiteral("qt-project.org"));

    QCommandLineParser parser;
    QCommandLineOption outputOption(
                QStringLiteral("output"),
                QStringLiteral("Write output to a file."),
                QStringLiteral("file"));
    parser.addOption(outputOption);
    QCommandLineOption verbosityOption(
                QStringLiteral("verbose"),
                QLatin1String("The verbosity level of the message output "
                              "(0 - silent, 1 - info, 2 - debug). Defaults to 1."),
                QStringLiteral("level"), QStringLiteral("1"));
    parser.addOption(verbosityOption);

    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.process(app.arguments());

    if (parser.isSet(outputOption)) {
        outputFile.setFileName(parser.value(outputOption));
        if (!outputFile.open(QFile::WriteOnly)) {
            qCWarning(lcD3DService) << "The output file could not be opened:"
                                    << outputFile.errorString();
            return 1;
        }
        qInstallMessageHandler(&outputFileMessageHandler);
    }

    QStringList filterRules = QStringList() // Default logging rules
            << QStringLiteral("qt.d3dservice.warning=true")
            << QStringLiteral("qt.d3dservice.critical=true");
    if (parser.isSet(verbosityOption)) {
        bool ok;
        uint verbosity = parser.value(verbosityOption).toUInt(&ok);
        if (!ok || verbosity > 2) {
            qCCritical(lcD3DService) << "Incorrect value specified for verbosity.";
            parser.showHelp(1);
        }
        switch (verbosity) {
        case 2: // Enable debug print
            filterRules.append(QStringLiteral("qt.d3dservice.debug=true"));
            break;
        case 1: // Remove warnings
            filterRules.removeFirst();
            // fall through
        case 0: // Silent
            filterRules.removeFirst();
            // fall through
        default: // Impossible
            break;
        }
    }
    QLoggingCategory::setFilterRules(filterRules.join(QLatin1Char('\n')));

    if (!D3DService::start()) {
        qCWarning(lcD3DService) << "The service failed to start.";
        return 1;
    }

    return 0;
}
