// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "runqttool.h"
#include "fmt.h"

#include "profileutils.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qregularexpression.h>

#include <cstdlib>
#include <iostream>

#ifdef Q_OS_UNIX
#include <sys/wait.h>
#endif

static QString qtToolFilePath(const QString &toolName, QLibraryInfo::LibraryPath location)
{
    QString filePath = QLibraryInfo::path(location) + QLatin1Char('/') + toolName;
#ifdef Q_OS_WIN
    filePath.append(QLatin1String(".exe"));
#endif
    return QDir::cleanPath(filePath);
}

static void rtPrintErr(const QString &out)
{
    std::cerr << qUtf8Printable(out);
}

static QString shellQuoted(const QString &str)
{
    static QRegularExpression rx(QStringLiteral("\\s"));
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

static int runQtToolHelper(const QString &toolName, const QStringList &arguments,
                           QLibraryInfo::LibraryPath location)
{
    int exitCode = 0;
    const QString commandLine = commandLineForSystem(qtToolFilePath(toolName, location), arguments);
#if defined(Q_OS_WIN)
    exitCode = _wsystem(reinterpret_cast<const wchar_t *>(commandLine.utf16()));
#elif defined(Q_OS_UNIX)
    int ret = std::system(qPrintable(commandLine));
    exitCode = WEXITSTATUS(ret);
#else
    exitCode = std::system(qPrintable(commandLine));
#endif
    return exitCode;
}

void runQtTool(const QString &toolName, const QStringList &arguments,
               QLibraryInfo::LibraryPath location)
{
    const int exitCode = runQtToolHelper(toolName, arguments, location);
    if (exitCode != 0)
        exit(exitCode);
}

static int runInternalQtToolHelper(const QString &toolName, const QStringList &arguments)
{
    return runQtToolHelper(toolName, arguments, QLibraryInfo::LibraryExecutablesPath);
}

void runInternalQtTool(const QString &toolName, const QStringList &arguments)
{
    const int exitCode = runInternalQtToolHelper(toolName, arguments);
    if (exitCode != 0)
        exit(exitCode);
}

std::unique_ptr<QTemporaryFile> createProjectDescription(QStringList args)
{
    std::unique_ptr<QTemporaryFile> file(new QTemporaryFile(QStringLiteral("XXXXXX.json")));
    if (!file->open()) {
        rtPrintErr(FMT::tr("Cannot create temporary file: %1\n").arg(file->errorString()));
        exit(1);
    }
    file->close();
    args << QStringLiteral("-out") << file->fileName();
    const int exitCode = runInternalQtToolHelper(QStringLiteral("lprodump"), args);
    if (exitCode != 0) {
        file.reset();
        exit(exitCode);
    }
    return file;
}
