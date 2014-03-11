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

#include "d3dservice.h"

#include <QtCore/QFile>

#include <wincred.h>
#include <ntsecapi.h>
#define SECURITY_WIN32
#include <security.h>

// Utility methods for getting user credentials and elevating the process
bool D3DService::getCredentials(LPWSTR username, DWORD *usernameSize,
                           LPWSTR password, DWORD *passwordSize)
{
    WCHAR user[MAX_PATH];
    ULONG userSize = MAX_PATH;
    GetUserNameEx(NameSamCompatible, user, &userSize);

    // Get LSA handle and package ID
    HANDLE lsaHandle;
    DWORD result = LsaConnectUntrusted(&lsaHandle);
    if (result != ERROR_SUCCESS) {
        qCWarning(lcD3DService) << "Unable to get LSA handle:"
                                << qt_error_string(LsaNtStatusToWinError(result));
        return false;
    }
    LSA_STRING packageName = { sizeof(NEGOSSP_NAME_A) - 1, sizeof(NEGOSSP_NAME_A), NEGOSSP_NAME_A };
    ULONG packageId;
    result = LsaLookupAuthenticationPackage(lsaHandle, &packageName, &packageId);
    if (result != ERROR_SUCCESS) {
        qCWarning(lcD3DService) << "Unable to get authentication package:"
                                << qt_error_string(LsaNtStatusToWinError(result));
        LsaClose(lsaHandle);
        return false;
    }
    LsaClose(lsaHandle);

    CREDUI_INFO uiInfo = { sizeof(CREDUI_INFO), GetForegroundWindow(),
                           L"Enter the credentials for your local user. Or, simply click OK "
                           L"and update the credentials later in the Services console.",
                           L"Qt D3D Compilation Service", NULL };
    QByteArray credentialsStorage(256, Qt::Uninitialized);
    BLOB credentials = { credentialsStorage.size(), (BYTE *)credentialsStorage.data() };
    if (!CredPackAuthenticationBuffer(CRED_PACK_GENERIC_CREDENTIALS, user, L"",
                                      credentials.pBlobData, &credentials.cbSize)) {
        qCWarning(lcD3DService) << "Unable to pack authentication buffer:"
                                << qt_error_string(GetLastError());
        return false;
    }

    BLOB authBuffer;
    result = CredUIPromptForWindowsCredentials(&uiInfo, 0, &packageId,
                                               credentials.pBlobData, credentials.cbSize,
                                               (LPVOID *)&authBuffer.pBlobData, &authBuffer.cbSize,
                                               NULL, CREDUIWIN_GENERIC);
    if (result == ERROR_SUCCESS) {
        WCHAR domain[MAX_PATH] = { 0 };
        DWORD domainSize = MAX_PATH;
        const BOOL unpackOk = CredUnPackAuthenticationBuffer(
                    0, authBuffer.pBlobData, authBuffer.cbSize,
                    username, usernameSize, domain, &domainSize,
                    password, passwordSize);
        if (unpackOk) {
            return true;
        } else {
            qCWarning(lcD3DService) << "Unable to unpack credentials:"
                                    << qt_error_string(GetLastError());
        }
    }

    qCWarning(lcD3DService) << qt_error_string(result);
    SecureZeroMemory(password, *passwordSize);
    SecureZeroMemory(username, *usernameSize);
    *usernameSize = *passwordSize = 0;
    return false;
}

bool D3DService::executeElevated(LPCWSTR exe, LPCWSTR param, DWORD *exitCode)
{
    // Create a temporary file for redirected output
    wchar_t tempPath[MAX_PATH];
    if (!GetTempPath(MAX_PATH, tempPath)) {
        qCWarning(lcD3DService) << "Unable to get the temporary file path for redirected output:"
                                << qt_error_string(GetLastError());
        return false;
    }
    wchar_t tempFileName[MAX_PATH];
    if (!GetTempFileName(tempPath, L"temp", 0, tempFileName)) {
        qCWarning(lcD3DService) << "Unable to get a temporary file name for redirected output:"
                                << qt_error_string(GetLastError());
        return false;
    }

    qCWarning(lcD3DService) << "Requesting administrative permissions...";
    const QString args = QString::fromWCharArray(param) + QStringLiteral(" -output \"")
            + QString::fromWCharArray(tempFileName) + QLatin1Char('"');
    SHELLEXECUTEINFO executeInfo = {
        sizeof(SHELLEXECUTEINFO), SEE_MASK_NOCLOSEPROCESS, GetConsoleWindow(),
        L"runas", exe, reinterpret_cast<LPCWSTR>(args.utf16()), 0, SW_HIDE
    };
    if (!ShellExecuteEx(&executeInfo)) {
        qCWarning(lcD3DService) << "Unable to elevate the process:"
                                << qt_error_string(GetLastError());
        return false;
    }

    qCWarning(lcD3DService) << "Elevation successful.";

    WaitForSingleObject(executeInfo.hProcess, INFINITE);

    // Fetch program output
    QFile output(QString::fromWCharArray(tempFileName));
    if (output.open(QFile::ReadOnly)) {
        while (!output.atEnd())
            qCWarning(lcD3DService) << output.readLine().trimmed().constData();
    } else {
        qCWarning(lcD3DService) << "Unable to open output file:"
                                << output.errorString();
    }
    if (!output.remove()) {
        qCWarning(lcD3DService) << "Unable to remove the output file:"
                                << output.errorString();
    }

    if (!GetExitCodeProcess(executeInfo.hProcess, exitCode)) {
        qCWarning(lcD3DService) << "Exit status unknown:"
                                << qt_error_string(GetLastError());
        CloseHandle(executeInfo.hProcess);
        return false;
    }
    CloseHandle(executeInfo.hProcess);

    return true;
}

bool D3DService::addLogonRight(LPCWSTR username)
{
    HANDLE lsaHandle;
    LSA_OBJECT_ATTRIBUTES attributes = { 0 };
    DWORD result = LsaOpenPolicy(NULL, &attributes, POLICY_LOOKUP_NAMES, &lsaHandle);
    if (result != ERROR_SUCCESS) {
        qCWarning(lcD3DService) << "Unable to get LSA handle:"
                                << qt_error_string(LsaNtStatusToWinError(result));
        return false;
    }

    char sid[96]; // Maximum binary size of SID should be 68; use 96 to be safe
    DWORD sidSize = sizeof(sid);
    SID_NAME_USE use;
    wchar_t domain[MAX_PATH];
    DWORD domainSize = MAX_PATH;
    if (!LookupAccountName(NULL, username, sid, &sidSize, domain, &domainSize, &use)) {
        qCWarning(lcD3DService) << "Unable to lookup account SID:"
                                << qt_error_string(GetLastError());
        LsaClose(lsaHandle);
        return false;
    }

    ushort len = wcslen(SE_SERVICE_LOGON_NAME) * sizeof(WCHAR);
    LSA_UNICODE_STRING rights = { len, len + sizeof(WCHAR), SE_SERVICE_LOGON_NAME };
    result = LsaAddAccountRights(lsaHandle, sid, &rights, 1);
    if (result != ERROR_SUCCESS) {
        qCWarning(lcD3DService) << "Unable to grant the user service logon rights:"
                                << qt_error_string(LsaNtStatusToWinError(result));
        LsaClose(lsaHandle);
        return false;
    }

    LsaClose(lsaHandle);
    return true;
}
