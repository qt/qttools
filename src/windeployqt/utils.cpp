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
#include <QtCore/QScopedPointer>
#include <QtCore/QScopedArrayPointer>
#include <QtCore/QStandardPaths>

#include <cstdio>
#include <Shlwapi.h>

int optVerboseLevel = 1;

// Find a file in the path using ShellAPI. This can be used to locate DLLs which
// QStandardPaths cannot do.
QString findInPath(const QString &file)
{
    if (file.size() < MAX_PATH -  1) {
        wchar_t buffer[MAX_PATH];
        file.toWCharArray(buffer);
        buffer[file.size()] = 0;
        if (PathFindOnPath(buffer, NULL))
            return QString::fromWCharArray(buffer);
    }
    return QString();
}

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

    STARTUPINFO myInfo;
    GetStartupInfo(&myInfo);
    si.hStdInput = myInfo.hStdInput;
    si.hStdOutput = myInfo.hStdOutput;
    si.hStdError = myInfo.hStdError;

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

    // Create a copy of the command line which CreateProcessW can modify.
    QScopedArrayPointer<wchar_t> commandLineW(new wchar_t[commandLine.size() + 1]);
    commandLine.toWCharArray(commandLineW.data());
    commandLineW[commandLine.size()] = 0;
    if (!CreateProcessW(0, commandLineW.data(), 0, 0, /* InheritHandles */ TRUE, 0, 0,
                        (wchar_t *)nativeWorkingDir.utf16(), &si, &pi)) {
        if (stdOut)
            CloseHandle(si.hStdOutput);
        if (stdErr)
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

QMap<QString, QString> queryQMakeAll(QString *errorMessage)
{
    QByteArray stdOut;
    QByteArray stdErr;
    unsigned long exitCode = 0;
    const QString commandLine = QStringLiteral("qmake.exe -query");
    if (!runProcess(commandLine, QString(), &exitCode, &stdOut, &stdErr, errorMessage))
        return QMap<QString, QString>();
    if (exitCode) {
        *errorMessage = commandLine + QStringLiteral(" returns ") + QString::number(exitCode)
            + QStringLiteral(": ") + QString::fromLocal8Bit(stdErr);
        return QMap<QString, QString>();
    }
    const QString output = QString::fromLocal8Bit(stdOut).trimmed();
    QMap<QString, QString> result;
    int pos = 0;
    while (true) {
        const int colonPos = output.indexOf(QLatin1Char(':'), pos);
        if (colonPos < 0)
            break;
        const int endPos = output.indexOf(QLatin1Char('\n'), colonPos + 1);
        if (endPos < 0)
            break;
        const QString key = output.mid(pos, colonPos - pos);
        const QString value = output.mid(colonPos + 1, endPos - colonPos - 2); // Skip '\r'
        result.insert(key, value);
        pos = endPos + 1;
    }
    return result;
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

// Update a file or directory.
bool updateFile(const QString &sourceFileName, const QStringList &nameFilters,
                const QString &targetDirectory, QString *errorMessage)
{
    const QFileInfo sourceFileInfo(sourceFileName);
    const QString targetFileName = targetDirectory + QLatin1Char('/') + sourceFileInfo.fileName();
    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Checking %s, %s\n", qPrintable(sourceFileName), qPrintable(targetFileName));

    if (!sourceFileInfo.exists()) {
        *errorMessage = QString::fromLatin1("%1 does not exist.").arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }

    if (sourceFileInfo.isSymLink()) {
        *errorMessage = QString::fromLatin1("Symbolic links are not supported (%1).")
                        .arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }

    const QFileInfo targetFileInfo(targetFileName);

    if (sourceFileInfo.isDir()) {
        if (targetFileInfo.exists()) {
            if (!targetFileInfo.isDir()) {
                *errorMessage = QString::fromLatin1("%1 already exists and is not a directory.")
                                .arg(QDir::toNativeSeparators(targetFileName));
                return false;
            } // Not a directory.
        } else { // exists.
            QDir d(targetDirectory);
            std::printf("Creating %s.\n", qPrintable( QDir::toNativeSeparators(targetFileName)));
            if (!d.mkdir(sourceFileInfo.fileName())) {
                *errorMessage = QString::fromLatin1("Cannot create directory %1 under %2.")
                                .arg(sourceFileInfo.fileName(), QDir::toNativeSeparators(targetDirectory));
                return false;
            }
        }
        // Recurse into directory
        QDir dir(sourceFileName);
        const QStringList allEntries = dir.entryList(nameFilters, QDir::Files) + dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &entry, allEntries)
            if (!updateFile(sourceFileName + QLatin1Char('/') + entry, nameFilters, targetFileName, errorMessage))
                return false;
        return true;
    } // Source is directory.

    if (targetFileInfo.exists()) {
        if (targetFileInfo.lastModified() >= sourceFileInfo.lastModified()) {
            if (optVerboseLevel)
                std::printf("%s is up to date.\n", qPrintable(sourceFileInfo.fileName()));
            return true;
        }
        QFile targetFile(targetFileName);
        if (!targetFile.remove()) {
            *errorMessage = QString::fromLatin1("Cannot remove existing file %1: %2")
                            .arg(QDir::toNativeSeparators(targetFileName), targetFile.errorString());
            return false;
        }
    } // target exists
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

// Helper for reading out PE executable files: Find a section header for an RVA
// (IMAGE_NT_HEADERS64, IMAGE_NT_HEADERS32).
template <class ImageNtHeader>
const IMAGE_SECTION_HEADER *findSectionHeader(DWORD rva, const ImageNtHeader *nTHeader)
{
    const IMAGE_SECTION_HEADER *section = IMAGE_FIRST_SECTION(nTHeader);
    const IMAGE_SECTION_HEADER *sectionEnd = section + nTHeader->FileHeader.NumberOfSections;
    for ( ; section < sectionEnd; ++section)
        if (rva >= section->VirtualAddress && rva < (section->VirtualAddress + section->Misc.VirtualSize))
                return section;
    return 0;
}

// Helper for reading out PE executable files: convert RVA to pointer (IMAGE_NT_HEADERS64, IMAGE_NT_HEADERS32).
template <class ImageNtHeader>
inline const void *rvaToPtr(DWORD rva, const ImageNtHeader *nTHeader, const void *imageBase)
{
    const IMAGE_SECTION_HEADER *sectionHdr = findSectionHeader(rva, nTHeader);
    if (!sectionHdr)
        return 0;
    const DWORD delta = sectionHdr->VirtualAddress - sectionHdr->PointerToRawData;
    return static_cast<const char *>(imageBase) + rva - delta;
}

// Helper for reading out PE executable files: return word size of a IMAGE_NT_HEADERS64, IMAGE_NT_HEADERS32
template <class ImageNtHeader>
inline unsigned ntHeaderWordSize(const ImageNtHeader *header)
{
    // defines IMAGE_NT_OPTIONAL_HDR32_MAGIC, IMAGE_NT_OPTIONAL_HDR64_MAGIC
    enum { imageNtOptionlHeader32Magic = 0x10b, imageNtOptionlHeader64Magic = 0x20b };
    if (header->OptionalHeader.Magic == imageNtOptionlHeader32Magic)
        return 32;
    if (header->OptionalHeader.Magic == imageNtOptionlHeader64Magic)
        return 64;
    return 0;
}

// Helper for reading out PE executable files: Retrieve the NT image header of an
// executable via the legacy DOS header.
static IMAGE_NT_HEADERS *getNtHeader(void *fileMemory, QString *errorMessage)
{
    IMAGE_DOS_HEADER *dosHeader = static_cast<PIMAGE_DOS_HEADER>(fileMemory);
    // Check DOS header consistency
    if (IsBadReadPtr(dosHeader, sizeof(IMAGE_DOS_HEADER))
        || dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        *errorMessage = QString::fromLatin1("DOS header check failed.");
        return 0;
    }
    // Retrieve NT header
    char *ntHeaderC = static_cast<char *>(fileMemory) + dosHeader->e_lfanew;
    IMAGE_NT_HEADERS *ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS *>(ntHeaderC);
    // check NT header consistency
    if (IsBadReadPtr(ntHeaders, sizeof(ntHeaders->Signature))
        || ntHeaders->Signature != IMAGE_NT_SIGNATURE
        || IsBadReadPtr(&ntHeaders->FileHeader, sizeof(IMAGE_FILE_HEADER))) {
        *errorMessage = QString::fromLatin1("NT header check failed.");
        return 0;
    }
    // Check magic
    if (!ntHeaderWordSize(ntHeaders)) {
        *errorMessage = QString::fromLatin1("NT header check failed; magic %1 is invalid.").
                        arg(ntHeaders->OptionalHeader.Magic);
        return 0;
    }
    // Check section headers
    IMAGE_SECTION_HEADER *sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);
    if (IsBadReadPtr(sectionHeaders, ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))) {
        *errorMessage = QString::fromLatin1("NT header section header check failed.");
        return 0;
    }
    return ntHeaders;
}

// Helper for reading out PE executable files: Read out import sections from
// IMAGE_NT_HEADERS64, IMAGE_NT_HEADERS32.
template <class ImageNtHeader>
inline QStringList readImportSections(const ImageNtHeader *ntHeaders, const void *base, QString *errorMessage)
{
    // Get import directory entry RVA and read out
    const DWORD importsStartRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!importsStartRVA) {
        *errorMessage = QString::fromLatin1("Failed to find IMAGE_DIRECTORY_ENTRY_IMPORT entry.");
        return QStringList();
    }
    const IMAGE_IMPORT_DESCRIPTOR *importDesc = (const IMAGE_IMPORT_DESCRIPTOR *)rvaToPtr(importsStartRVA, ntHeaders, base);
    if (!importDesc) {
        *errorMessage = QString::fromLatin1("Failed to find IMAGE_IMPORT_DESCRIPTOR entry.");
        return QStringList();
    }
    QStringList result;
    for ( ; importDesc->Name; ++importDesc)
        result.push_back(QString::fromLocal8Bit((const char *)rvaToPtr(importDesc->Name, ntHeaders, base)));
    return result;
}

// Read a PE executable and determine dependent libraries, word size
// and debug flags. Note that the debug flag cannot be relied on for MinGW.
bool readPeExecutable(const QString &peExecutableFileName, QString *errorMessage,
                      QStringList *dependentLibrariesIn, unsigned *wordSizeIn,
                      bool *isDebugIn)
{
    bool result = false;
    HANDLE hFile = NULL;
    HANDLE hFileMap = NULL;
    void *fileMemory = 0;

    if (dependentLibrariesIn)
        dependentLibrariesIn->clear();
    if (wordSizeIn)
        *wordSizeIn = 0;
    if (isDebugIn)
        *isDebugIn = false;

    do {
        // Create a memory mapping of the file
        hFile = CreateFile(reinterpret_cast<const WCHAR*>(peExecutableFileName.utf16()), GENERIC_READ, FILE_SHARE_READ, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
            *errorMessage = QString::fromLatin1("Cannot open '%1': %2").arg(peExecutableFileName, winErrorMessage(GetLastError()));
            break;
        }

        hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hFileMap == NULL) {
            *errorMessage = QString::fromLatin1("Cannot create file mapping of '%1': %2").arg(peExecutableFileName, winErrorMessage(GetLastError()));
            break;
        }

        fileMemory = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
        if (!fileMemory) {
            *errorMessage = QString::fromLatin1("Cannot map '%1': %2").arg(peExecutableFileName, winErrorMessage(GetLastError()));
            break;
        }

        const IMAGE_NT_HEADERS *ntHeaders = getNtHeader(fileMemory, errorMessage);
        if (!ntHeaders)
            break;

        const unsigned wordSize = ntHeaderWordSize(ntHeaders);
        if (wordSizeIn)
            *wordSizeIn = wordSize;
        bool debug = false;
        if (wordSize == 32) {
            const IMAGE_NT_HEADERS32 *ntHeaders32 = reinterpret_cast<const IMAGE_NT_HEADERS32 *>(ntHeaders);
            debug = ntHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
            if (dependentLibrariesIn)
                *dependentLibrariesIn = readImportSections(ntHeaders32, fileMemory, errorMessage);
        } else {
            const IMAGE_NT_HEADERS64 *ntHeaders64 = reinterpret_cast<const IMAGE_NT_HEADERS64 *>(ntHeaders);
            debug = ntHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
            if (dependentLibrariesIn)
                *dependentLibrariesIn = readImportSections(ntHeaders64, fileMemory, errorMessage);
        }

        if (isDebugIn)
            *isDebugIn = debug;
        result = true;
        if (optVerboseLevel > 1)
            std::fprintf(stderr, "%s: %s %u bit, debug: %d\n", __FUNCTION__,
                         qPrintable(peExecutableFileName), wordSize, debug);
    } while (false);

    if (fileMemory)
        UnmapViewOfFile(fileMemory);

    if (hFileMap != NULL)
        CloseHandle(hFileMap);

    if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return result;
}

QT_END_NAMESPACE
