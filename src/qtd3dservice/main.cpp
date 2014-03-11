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
    QCommandLineOption installOption(
                QStringLiteral("install"),
                QStringLiteral("Install the Qt D3D shader compilation service."));
    parser.addOption(installOption);
    QCommandLineOption removeOption(
                QStringLiteral("remove"),
                QStringLiteral("Uninstall the Qt D3D shader compilation service."));
    parser.addOption(removeOption);
    QCommandLineOption registerOption(
                QStringLiteral("register"),
                QStringLiteral("Register an application for monitoring."),
                QStringLiteral("[device_id:]application_id"));
    parser.addOption(registerOption);
    QCommandLineOption unregisterOption(
                QStringLiteral("unregister"),
                QStringLiteral("Unregister an application for monitoring."),
                QStringLiteral("[device_id:]application_id"));
    parser.addOption(unregisterOption);
    QCommandLineOption listOption(
                QStringLiteral("list"),
                QStringLiteral("List the registered applications."));
    parser.addOption(listOption);
    QCommandLineOption clearOption(
                QStringLiteral("clear"),
                QStringLiteral("Removes all registered applications."));
    parser.addOption(clearOption);
    QCommandLineOption directOption(
                QStringLiteral("direct"),
                QStringLiteral("Run directly (not as a Windows service)."));
    parser.addOption(directOption);
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

    const bool installSet = parser.isSet(installOption);
    const bool removeSet = parser.isSet(removeOption);
    const bool registerSet = parser.isSet(registerOption);
    const bool unregisterSet = parser.isSet(unregisterOption);
    const bool listSet = parser.isSet(listOption);
    const bool clearSet = parser.isSet(clearOption);
    const bool directSet = parser.isSet(directOption);
    // All options are mutually exclusive
    if (installSet + removeSet + registerSet + unregisterSet + listSet + clearSet + directSet > 1)
        parser.showHelp(1);

    if (installSet)
        return D3DService::install() ? 0 : 1;

    if (removeSet)
        return D3DService::remove() ? 0 : 1;

    if (clearSet)
        return D3DService::clearRegistrations() ? 0 : 1;

    if (listSet) {
        std::wcout << "Registered applications:" << std::endl;
        QList<QStringPair> registrations = D3DService::registrations();
        foreach (const QStringPair &registration, registrations) {
            std::wcout << "  " << reinterpret_cast<const wchar_t *>(registration.first.utf16())
                       << "  " << reinterpret_cast<const wchar_t *>(registration.second.utf16()) << std::endl;
        }
        return 0;
    }

    if (registerSet || unregisterSet) {
        QStringList parts = parser.value(registerSet ? registerOption : unregisterOption).split(QLatin1Char(':'));
        if (parts.size() == 1)
            parts.prepend(QStringLiteral("local"));
        if (parts.size() != 2)
            parser.showHelp(1);

        // Validation?

        if (registerSet) {
            if (D3DService::registerApp(parts.at(0), parts.at(1))) {
                qCWarning(lcD3DService) << "Registered application.";
                return 0;
            }
            qCWarning(lcD3DService) << "Unable to register application.";
            return 1;
        }

        if (unregisterSet) {
            if (D3DService::unregisterApp(parts.at(0), parts.at(1))) {
                qCWarning(lcD3DService) << "Unregistered application.";
                return 0;
            }
            qCWarning(lcD3DService) << "Unable to unregister application.";
            return 1;
        }
    }

    if (directSet) {
        if (D3DService::startDirectly())
            return 0;
    }

    if (D3DService::startService(!parser.isSet(outputOption)))
        return 0;

    // App was launched directly without -direct
    parser.showHelp(1);
}
