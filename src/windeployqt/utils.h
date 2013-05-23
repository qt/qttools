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

#ifndef UTILS_H
#define UTILS_H

#include <QStringList>
#include <QMap>

enum Platform { Windows, WinRt };

QString findInPath(const QString &file);
QString normalizeFileName(const QString &name);
QString findSdkTool(const QString &tool);
QMap<QString, QString> queryQMakeAll(QString *errorMessage);
QString queryQMake(const QString &variable, QString *errorMessage);
QStringList findDependentLibs(const QString &binary, QString *errorMessage);

enum QtModule {
    GuiModule = 0x1,
    SqlModule = 0x4,
    NetworkModule = 0x8,
    MultimediaModule = 0x10,
    PrintSupportModule = 0x20,
    Quick1Module = 0x40,
    Quick2Module = 0x80,
    SensorsModule = 0x100,
    WebKitModule = 0x400
};

QStringList findQtPlugins(unsigned usedQtModules, bool debug, Platform platform,
                          QString *platformPlugin, QString *errorMessage);
bool updateFile(const QString &sourceFileName, const QStringList &nameFilters,
                const QString &targetDirectory, QString *errorMessage);
inline bool updateFile(const QString &sourceFileName, const QString &targetDirectory, QString *errorMessage)
{ return updateFile(sourceFileName, QStringList(), targetDirectory, errorMessage); }
bool runProcess(const QString &commandLine, const QString &workingDirectory = QString(),
                unsigned long *exitCode = 0, QByteArray *stdOut = 0, QByteArray *stdErr = 0,
                QString *errorMessage = 0);
QString winErrorMessage(unsigned long error);

bool readPeExecutable(const QString &peExecutableFileName, QString *errorMessage,
                      QStringList *dependentLibraries = 0, unsigned *wordSize = 0,
                      bool *isDebug = 0);
// Return dependent modules of a PE executable files.

inline QStringList findDependentLibraries(const QString &peExecutableFileName, QString *errorMessage)
{
    QStringList result;
    readPeExecutable(peExecutableFileName, errorMessage, &result);
    return result;
}

extern int optVerboseLevel;

#endif // UTILS_H
