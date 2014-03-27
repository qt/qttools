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
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamWriter>

#include <iostream>

#include "d3dservice.h"

static QFile outputFile;
static void outputFileMessageHandler(QtMsgType, const QMessageLogContext &, const QString &text)
{
    outputFile.write(text.toUtf8());
    outputFile.write("\r\n");
}

static void outputList(const QStringList &files, bool useLogger)
{
    foreach (const QString &file, files) {
        if (useLogger)
            qCCritical(lcD3DService, qPrintable(file));
        else
            std::wcout << reinterpret_cast<const wchar_t *>(file.utf16()) << std::endl;
    }
}

static void outputQrc(const QStringList &files, bool useLogger)
{
    QString stream;
    QXmlStreamWriter xml(&stream);
    xml.setAutoFormatting(true);
    xml.writeStartElement(QStringLiteral("RCC"));
    xml.writeStartElement(QStringLiteral("qresource"));
    xml.writeAttribute(QStringLiteral("prefix"), QStringLiteral("qt.d3dcompiler"));
    foreach (const QString &file, files) {
        xml.writeStartElement(QStringLiteral("file"));
        xml.writeAttribute(QStringLiteral("alias"), QFileInfo(file).fileName());
        xml.writeCharacters(file);
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();

    if (useLogger)
        qCCritical(lcD3DService, qPrintable(stream));
    else
        std::wcout << reinterpret_cast<const wchar_t *>(stream.utf16()) << std::endl;
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
    QCommandLineOption listSourceOption(
                QStringLiteral("list-source"),
                QLatin1String("List the known shader sources. Use with --app and/or --device "
                              "to narrow the scope."));
    parser.addOption(listSourceOption);
    QCommandLineOption listBinaryOption(
                QStringLiteral("list-binary"),
                QLatin1String("List the known shader binaries. Use with --app and/or --device "
                              "to narrow the scope."));
    parser.addOption(listBinaryOption);
    QCommandLineOption appOption(
                QStringLiteral("app"),
                QStringLiteral("Specifies the application to act upon."),
                QStringLiteral("name"));
    parser.addOption(appOption);
    QCommandLineOption deviceOption(
                QStringLiteral("device"),
                QStringLiteral("Specifies the device to act upon."),
                QStringLiteral("name"));
    parser.addOption(deviceOption);
    QCommandLineOption qrcOption(
                QStringLiteral("qrc"),
                QLatin1String("Outputs the content of --list-source/--list-binary in "
                              "Qt resource file format."));
    parser.addOption(qrcOption);

    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.process(app.arguments());


    const bool useLogger = parser.isSet(outputOption);
    const bool useQrc = parser.isSet(qrcOption);
    const bool listSource = parser.isSet(listSourceOption);
    const bool listBinary = parser.isSet(listBinaryOption);

    if (useLogger) {
        outputFile.setFileName(parser.value(outputOption));
        if (!outputFile.open(QFile::WriteOnly)) {
            qCWarning(lcD3DService) << "The output file could not be opened:"
                                    << outputFile.errorString();
            return 1;
        }
        qInstallMessageHandler(&outputFileMessageHandler);
    }

    if (useQrc && !(listSource || listBinary)) {
        qCWarning(lcD3DService) << "The --qrc option is only valid with either --list-source or --list--binary.";
        return 1;
    }

    if (listSource && listBinary) {
        qCWarning(lcD3DService) << "Please specify only --list-binary or --list source, not both.";
        return 1;
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
    } else if (listSource || listBinary) {
        // In list mode, silence warnings by default
        filterRules.removeFirst();
    }
    QLoggingCategory::setFilterRules(filterRules.join(QLatin1Char('\n')));

    const QString deviceName = parser.value(deviceOption);
    const QString appName = parser.value(appOption);
    QStringList devices;
    if (listSource || listBinary)
        devices = deviceName.isEmpty() ? D3DService::devices() : QStringList(deviceName);

    if (listSource) {
        foreach (const QString &device, devices) {
            const QStringList apps = appName.isEmpty()
                    ? D3DService::apps(device) : QStringList(appName);
            foreach (const QString &app, apps) {
                const QStringList files = D3DService::sources(device, app);
                if (useQrc)
                    outputQrc(files, useLogger);
                else
                    outputList(files, useLogger);
            }
        }

        return 0;
    }

    if (listBinary) {
        foreach (const QString &device, devices) {
            const QStringList apps = appName.isEmpty()
                    ? D3DService::apps(device) : QStringList(appName);
            foreach (const QString &app, apps) {
                const QStringList files = D3DService::binaries(device, app);
                if (useQrc)
                    outputQrc(files, useLogger);
                else
                    outputList(files, useLogger);
            }
        }

        return 0;
    }

    // Default (start service)
    if (!D3DService::start()) {
        qCWarning(lcD3DService) << "The service failed to start.";
        return 1;
    }

    return 0;
}
