/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "runqttool.h"

#include "profileutils.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qregexp.h>

#include <cstdlib>
#include <iostream>

#ifdef Q_OS_UNIX
#include <sys/wait.h>
#endif

class FMT {
    Q_DECLARE_TR_FUNCTIONS(Linguist)
};

static QString qtToolFilePath(const QString &toolName)
{
    QString filePath = QCoreApplication::instance()->applicationDirPath()
            + QLatin1Char('/') + toolName;
#ifdef Q_OS_WIN
    filePath.append(QLatin1String(".exe"));
#endif
    return QDir::cleanPath(filePath);
}

static void printErr(const QString &out)
{
    std::cerr << qUtf8Printable(out);
}

static QString shellQuoted(const QString &str)
{
    static QRegExp rx(QStringLiteral("\\s"));
    QString result = str;
    if (result.contains(rx)) {
        const QLatin1Char dblqt = QLatin1Char('"');
        result.prepend(dblqt);
        result.append(dblqt);
    }
    return result;
}

static QStringList shellQuoted(const QStringList &strs)
{
    QStringList result;
    result.reserve(strs.size());
    std::transform(strs.begin(), strs.end(), std::back_inserter(result),
                   static_cast<QString (*)(const QString &)>(&shellQuoted));
    return result;
}

static QString commandLineForSystem(const QString &program,
                                    const QStringList &arguments)
{
    return shellQuoted(program)
            + QLatin1Char(' ')
            + shellQuoted(arguments).join(QLatin1Char(' '));
}

void runQtTool(const QString &toolName, const QStringList &arguments)
{
    int exitCode = 0;
    const QString commandLine = commandLineForSystem(qtToolFilePath(toolName), arguments);
#if defined(Q_OS_WIN)
    exitCode = _wsystem(reinterpret_cast<const wchar_t *>(commandLine.utf16()));
#elif defined(Q_OS_UNIX)
    int ret = std::system(qPrintable(commandLine));
    exitCode = WEXITSTATUS(ret);
#else
    exitCode = std::system(qPrintable(commandLine));
#endif
    if (exitCode != 0)
        exit(exitCode);
}

std::unique_ptr<QTemporaryFile> createProjectDescription(QStringList args)
{
    std::unique_ptr<QTemporaryFile> file(new QTemporaryFile(QStringLiteral("XXXXXX.json")));
    if (!file->open()) {
        printErr(FMT::tr("Cannot create temporary file: %1\n").arg(file->errorString()));
        exit(1);
    }
    file->close();
    args << QStringLiteral("-out") << file->fileName();
    runQtTool(QStringLiteral("lprodump"), args);
    return file;
}
