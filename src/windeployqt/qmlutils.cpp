/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include "qmlutils.h"
#include "utils.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonParseError>

QT_BEGIN_NAMESPACE

static QString qmlDirectoryRecursion(Platform platform, const QString &path)
{
    QDir dir(path);
    if (!dir.entryList(QStringList(QStringLiteral("*.qml")), QDir::Files, QDir::NoSort).isEmpty())
        return dir.path();
    foreach (const QString &subDir, dir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort)) {
        if (!isBuildDirectory(platform, subDir)) {
            const QString subPath = qmlDirectoryRecursion(platform, dir.path() +  QLatin1Char('/') + subDir);
            if (!subPath.isEmpty())
                return subPath;
        }
    }
    return QString();
}

// Find a directory containing QML files in the project
QString findQmlDirectory(int platform, const QString &startDirectoryName)
{
    QDir startDirectory(startDirectoryName);
    if (isBuildDirectory(Platform(platform), startDirectory.dirName()))
        startDirectory.cdUp();
    return qmlDirectoryRecursion(Platform(platform), startDirectory.path());
}

static void findFileRecursion(const QDir &directory, Platform platform, bool debug, QStringList *matches)
{
    foreach (const QString &dll, findSharedLibraries(directory, platform, debug))
        matches->append(directory.filePath(dll));
    foreach (const QString &subDir, directory.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
        QDir subDirectory = directory;
        if (subDirectory.cd(subDir))
            findFileRecursion(subDirectory, platform, debug, matches);
    }
}

QmlImportScanResult runQmlImportScanner(const QString &directory, const QString &qmlImportPath,
                                        int platform, bool debug, QString *errorMessage)
{
    QmlImportScanResult result;
    QStringList arguments;
    arguments << QStringLiteral("-importPath") << qmlImportPath << QStringLiteral("-rootPath") << directory;
    unsigned long exitCode;
    QByteArray stdOut;
    QByteArray stdErr;
    const QString binary = QStringLiteral("qmlimportscanner");
    if (!runProcess(binary, arguments, QDir::currentPath(), &exitCode, &stdOut, &stdErr, errorMessage))
        return result;
    if (exitCode) {
        *errorMessage = binary + QStringLiteral(" returned ") + QString::number(exitCode)
                        + QStringLiteral(": ") + QString::fromLocal8Bit(stdErr);
        return result;
    }
    QJsonParseError jsonParseError;
    const QJsonDocument data = QJsonDocument::fromJson(stdOut, &jsonParseError);
    if (data.isNull() ) {
        *errorMessage = binary + QStringLiteral(" returned invalid JSON output: ")
                        + jsonParseError.errorString() + QStringLiteral(" :\"")
                        + QString::fromLocal8Bit(stdOut) + QLatin1Char('"');
        return result;
    }
    const QJsonArray array = data.array();
    const int childCount = array.count();
    for (int c = 0; c < childCount; ++c) {
        const QJsonObject object = array.at(c).toObject();
        if (object.value(QStringLiteral("type")).toString() == QLatin1String("module")) {
            const QString path = object.value(QStringLiteral("path")).toString();
            if (!path.isEmpty()) {
                result.modulesDirectories.append(path);
                findFileRecursion(QDir(path), Platform(platform), debug, &result.plugins);
            }
        }
    }
    result.ok = true;
    return result;
}

QT_END_NAMESPACE
