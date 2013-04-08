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

#include "utils.h"

#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/qt_windows.h>
#include <QtCore/QTemporaryFile>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QScopedPointer>
#include <QtCore/QDateTime>
#include <QtCore/QStandardPaths>

#include <cstdio>

int optVerboseLevel = 1;

QString winErrorMessage(unsigned long error)
{
    QString rc = QString::fromLatin1("#%1: ").arg(error);
    ushort *lpMsgBuf;

    const int len = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, 0, (LPTSTR)&lpMsgBuf, 0, NULL);
    if (len) {
        rc = QString::fromUtf16(lpMsgBuf, len);
        LocalFree(lpMsgBuf);
    } else {
        rc += QString::fromLatin1("<unknown error>");
    }
    return rc;
}

// Case-Normalize file name via GetShortPathNameW()/GetLongPathNameW()
QString normalizeFileName(const QString &name)
{
    wchar_t shortBuffer[MAX_PATH];
    const QString nativeFileName = QDir::toNativeSeparators(name);
    if (!GetShortPathNameW((wchar_t *)nativeFileName.utf16(), shortBuffer, MAX_PATH))
        return name;
    wchar_t result[MAX_PATH];
    if (!GetLongPathNameW(shortBuffer, result, MAX_PATH))
        return name;
    return QDir::fromNativeSeparators(QString::fromWCharArray(result));
}

// Find a tool binary in the Windows SDK 8
QString findSdkTool(const QString &tool)
{
    QStringList paths = QString::fromLocal8Bit(qgetenv("PATH")).split(QLatin1Char(';'));
    const QByteArray sdkDir = qgetenv("WindowsSdkDir");
    if (!sdkDir.isEmpty())
        paths.prepend(QDir::cleanPath(QString::fromLocal8Bit(sdkDir)) + QLatin1String("/Tools/x64"));
    return QStandardPaths::findExecutable(tool, paths);
}

// runProcess helper: Create a temporary file for stdout/stderr redirection.
static HANDLE createInheritableTemporaryFile()
{
    wchar_t path[MAX_PATH];
    if (!GetTempPath(MAX_PATH, path))
        return INVALID_HANDLE_VALUE;
    wchar_t name[MAX_PATH];
    if (!GetTempFileName(path, L"temp", 0, name)) // Creates file.
        return INVALID_HANDLE_VALUE;
    SECURITY_ATTRIBUTES securityAttributes;
    ZeroMemory(&securityAttributes, sizeof(securityAttributes));
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;
    return CreateFile(name, GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE, &securityAttributes,
                      TRUNCATE_EXISTING, FILE_ATTRIBUTE_TEMPORARY, NULL);
}

// runProcess helper: Rewind and read out a temporary file for stdout/stderr.
static inline void readTemporaryProcessFile(HANDLE handle, QByteArray *result)
{
    if (SetFilePointer(handle, 0, 0, FILE_BEGIN) == 0xFFFFFFFF)
        return;
    char buf[1024];
    DWORD bytesRead;
    while (ReadFile(handle, buf, sizeof(buf), &bytesRead, NULL) && bytesRead)
        result->append(buf, bytesRead);
    CloseHandle(handle);
}

// runProcess: Run a command line process (replacement for QProcess which
// does not exist in the bootstrap library).
bool runProcess(const QString &commandLine, const QString &workingDirectory,
                unsigned long *exitCode, QByteArray *stdOut, QByteArray *stdErr,
                QString *errorMessage)
{
    if (exitCode)
        *exitCode = 0;

    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Running: %s\n", qPrintable(commandLine));

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdError = si.hStdOutput = si.hStdInput = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    const QChar backSlash = QLatin1Char('\\');
    QString nativeWorkingDir = QDir::toNativeSeparators(workingDirectory.isEmpty() ?  QDir::currentPath() : workingDirectory);
    if (!nativeWorkingDir.endsWith(backSlash))
        nativeWorkingDir += backSlash;

    if (stdOut) {
        si.hStdOutput = createInheritableTemporaryFile();
        if (si.hStdOutput == INVALID_HANDLE_VALUE) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Error creating stdout temporary file");
            return false;
        }
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    if (stdErr) {
        si.hStdError = createInheritableTemporaryFile();
        if (si.hStdError == INVALID_HANDLE_VALUE) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Error creating stderr temporary file");
            return false;
        }
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    if (!CreateProcessW(0, (WCHAR*)commandLine.utf16(), 0, 0, /* InheritHandles */ TRUE, 0, 0, 0, &si, &pi)) {
        if (si.hStdOutput != INVALID_HANDLE_VALUE)
            CloseHandle(si.hStdOutput);
        if (si.hStdError != INVALID_HANDLE_VALUE)
            CloseHandle(si.hStdError);
        if (errorMessage)
            *errorMessage = QStringLiteral("CreateProcessW failed: ") + winErrorMessage(GetLastError());
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    if (exitCode)
        GetExitCodeProcess(pi.hProcess, exitCode);
    CloseHandle(pi.hProcess);

    if (stdOut)
        readTemporaryProcessFile(si.hStdOutput, stdOut);
    if (stdErr)
        readTemporaryProcessFile(si.hStdError, stdErr);
    return true;
}

QString queryQMake(const QString &variable, QString *errorMessage)
{
    QByteArray stdOut;
    QByteArray stdErr;
    unsigned long exitCode;
    const QString commandLine = QStringLiteral("qmake.exe -query ") + variable;
    if (!runProcess(commandLine, QString(), &exitCode, &stdOut, &stdErr, errorMessage))
        return QString();
    if (exitCode) {
        *errorMessage = commandLine + QStringLiteral(" returns ") + QString::number(exitCode)
            + QStringLiteral(": ") + QString::fromLocal8Bit(stdErr);
        return QString();
    }
    return QString::fromLocal8Bit(stdOut).trimmed();
}

// run "depends.exe" in command line mode to find dependent libs.
// Note that this returns non-normalized (all CAPS) file names.
QStringList findDependentLibs(const QString &binary, QString *errorMessage)
{
    const QString depends = QStringLiteral("depends.exe");
    const QString dependsPath = findSdkTool(depends);
    if (dependsPath.isEmpty()) {
        *errorMessage = QString::fromLatin1("Cannot find %1.").arg(depends);
        return QStringList();
    }
    QString tempPattern = QDir::tempPath();
    if (!tempPattern.endsWith(QLatin1Char('/')))
        tempPattern += QLatin1Char('/');
    tempPattern += QLatin1String("dependsXXXXXX.csv");
    QScopedPointer<QTemporaryFile> temporaryFile(new QTemporaryFile(tempPattern));
    if (!temporaryFile->open()) {
        *errorMessage = QString::fromLatin1("Cannot open temporary file.");
        return QStringList();
    }
    const QString fileName = temporaryFile->fileName();
    temporaryFile->close();
    temporaryFile.reset();

    const QString commandLine = QLatin1Char('"') + QDir::toNativeSeparators(dependsPath)
        + QStringLiteral("\" /c /f:1 \"/oc:") + QDir::toNativeSeparators(fileName)
        +  QStringLiteral("\" \"") + QDir::toNativeSeparators(binary) + QLatin1Char('"');

    if (!runProcess(commandLine, QString(), 0, 0, 0, errorMessage))
        return QStringList();
    // Ignore return codes by missing dependencies (iexplorer,etc).
    QFile resultFile(fileName);
    if (!resultFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
        *errorMessage = QString::fromLatin1("Cannot open %1: %2")
                        .arg(QDir::toNativeSeparators(fileName), resultFile.errorString());
        return QStringList();
    }
    // Read CSV lines: 0,"c:\x.dll",...'
    QStringList result;
    resultFile.readLine(); // Skip header
    for (int l = 0; ; ++l) {
        const QByteArray line = resultFile.readLine();
        if (line.isEmpty() )
            break;
        if (l > 1) { // Skip header and exe
            int start = line.indexOf(',');
            if (start < 0)
                continue;
            ++start;
            if (line.at(start) == '"')
                ++start;
            int end = line.indexOf(',', start + 1);
            if (end < 0)
                continue;
            if (line.at(end - 1) == '"')
                --end;
            result.push_back(QDir::cleanPath(QString::fromLocal8Bit(line.mid(start, end - start))));
        }
    }
    return result;
}

QStringList findQtPlugins(bool debug, QString *errorMessage)
{
    const QString qtPluginsDirName = queryQMake(QStringLiteral("QT_INSTALL_PLUGINS"), errorMessage);
    if (qtPluginsDirName.isEmpty())
        return QStringList();
    QDir pluginsDir(qtPluginsDirName);
    QStringList result;
    const QString filter = QLatin1Char('*') + QLatin1String(debug ? "d" : "[^d]") + QStringLiteral(".dll");
    foreach (const QString &subDirName, pluginsDir.entryList(QStringList(QLatin1String("*")), QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (subDirName != QLatin1String("designer")) {
            const QString subDirPath = qtPluginsDirName + QLatin1Char('/') + subDirName;
            QDir subDir(subDirPath);
            // Use only the 'winrt' plugin from "platforms".
            const QString effectiveFilter = subDirName == QLatin1String("platforms") ?
                (QStringLiteral("qwinrt") + filter) : filter;
            foreach (const QString &dll, subDir.entryList(QStringList(effectiveFilter), QDir::Files))
                result.push_back(subDirPath + QLatin1Char('/') + dll);
        }
    }
    return result;
}

// Update a file
bool updateFile(const QString &sourceFileName, const QString &targetDirectory, QString *errorMessage)
{
    const QFileInfo sourceFileInfo(sourceFileName);
    const QString targetFileName = targetDirectory + QLatin1Char('/') + sourceFileInfo.fileName();
    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Checking %s, %s\n", qPrintable(sourceFileName), qPrintable(targetFileName));
    if (!sourceFileInfo.exists()) {
        *errorMessage = QString::fromLatin1("%1 does not exist.").arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }
    const QFileInfo targetFileInfo(targetFileName);
    if (targetFileInfo.exists() && targetFileInfo.lastModified() >= sourceFileInfo.lastModified()) {
        if (optVerboseLevel)
            std::printf("%s is up to date.\n", qPrintable(sourceFileInfo.fileName()));
        return true;
    }
    QFile file(sourceFileName);
    if (optVerboseLevel)
        std::printf("Updating %s.\n", qPrintable(sourceFileInfo.fileName()));
    if (!file.copy(targetFileName)) {
        *errorMessage = QString::fromLatin1("Cannot copy %1 to %2: %3")
                .arg(QDir::toNativeSeparators(sourceFileName),
                     QDir::toNativeSeparators(targetFileName),
                     file.errorString());
        return false;
    }
    return true;
}

QT_END_NAMESPACE
