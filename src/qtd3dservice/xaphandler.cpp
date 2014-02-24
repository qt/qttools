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

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>

#include <comdef.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#include <ccapi.h>
#include <corecon.h>
Q_GLOBAL_STATIC(CoreConServer, coreConServer)

#define bstr(s) _bstr_t((const wchar_t *)s.utf16())

/* This method runs in its own thread for each CoreCon device/application combo
 * the service is currently handling. */
extern void handleXapDevice(int deviceIndex, const QString &app, const QString &localBase)
{
    if (!coreConServer->initialize()) {
        while (!coreConServer.exists())
            Sleep(1);
    }

    CoreConDevice *device = coreConServer->devices().value(deviceIndex, 0);
    if (!device) {
        qCWarning(lcD3DService) << "Device at index" << deviceIndex << "not found.";
        return;
    }

    const QString localControlFile = localBase + QStringLiteral("\\control");
    const QString localSourcePath = localBase + QStringLiteral("\\source\\");
    const QString localBinaryPath = localBase + QStringLiteral("\\binary\\");

    const QString remoteBase = QStringLiteral("%FOLDERID_APPID_ISOROOT%\\")
                    + app + QStringLiteral("\\d3dcompiler");
    const QString remoteControlFile = remoteBase + QStringLiteral("\\control");
    const QString remoteSourcePath = remoteBase + QStringLiteral("\\source\\");
    const QString remoteBinaryPath = remoteBase + QStringLiteral("\\binary\\");

    HRESULT hr;
    _bstr_t connectionName;
    ComPtr<ICcConnection> connection;
    hr = coreConServer->handle()->GetConnection(
        device->handle(), 5000, NULL, connectionName.GetAddress(), &connection);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to initialize connection."
                                << coreConServer->formatError(hr);
        return;
    }

    ComPtr<ICcConnection3> connection3;
    hr = connection.As(&connection3);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to obtain connection3 interface."
                                << coreConServer->formatError(hr);
        return;
    }

    ComPtr<ICcConnection4> connection4;
    hr = connection.As(&connection4);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to obtain connection4 interface."
                                << coreConServer->formatError(hr);
        return;
    }

    D3DService::reportStatus(SERVICE_RUNNING, NO_ERROR, 0);

    int round = 0;
    bool wasDisconnected = true;
    FileInfo controlFileInfo;
    forever {
        VARIANT_BOOL connected;
        hr = connection->IsConnected(&connected);
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << "Unable to query connection state."
                                    << coreConServer->formatError(hr);
            Sleep(1000);
            continue;
        }

        int retryCount = 0;
        while (!connected) {
            hr = connection->ConnectDevice();
            connected = SUCCEEDED(hr);
            if (connected) {
                qCDebug(lcD3DService) << "Connected.";
                wasDisconnected = true;
            }
            if (FAILED(hr)) {
                qCDebug(lcD3DService) << "Unable to connect to device. Retrying..."
                                      << coreConServer->formatError(hr);
                if (++retryCount > 30) {
                    qCCritical(lcD3DService) << "Unable to connect to device. Exiting.";
                    return;
                }
                Sleep(1000);
            }
        }

        VARIANT_BOOL isInstalled = false;
        retryCount = 0;
        while (!isInstalled) {
            hr = connection3->IsApplicationInstalled(bstr(app), &isInstalled);
            if (FAILED(hr)) {
                qCCritical(lcD3DService) << "Unable to determine if package is installed - check that the app option contains a valid UUID."
                                         << coreConServer->formatError(hr);
                return;
            }

            if (!isInstalled) {
                qCDebug(lcD3DService) << "Package is not installed. Checking again...";
                if (++retryCount > 30) {
                    qCCritical(lcD3DService) << "Package did not materialize within 30 seconds. Exiting.";
                    return;
                }
                Sleep(1000);
            }
        }

        // Run certain setup steps once per connection
        if (wasDisconnected) {
            // Check if control file exists
            hr = connection->GetFileInfo(bstr(remoteControlFile), &controlFileInfo);
            if (FAILED(hr)) {
                if (hr != 0x80070003 /* Not found */) {
                    qCWarning(lcD3DService) << "Unable to obtain file info"
                                            << coreConServer->formatError(hr);
                    Sleep(1000);
                    continue;
                }
                // Not found, so let's upload it
                hr = connection->SendFile(bstr(localControlFile), bstr(remoteControlFile), 2, 2, NULL);
                if (FAILED(hr)) {
                    qCWarning(lcD3DService) << "Unable to send control file."
                                            << coreConServer->formatError(hr);
                    wasDisconnected = true;
                    Sleep(1000);
                    continue;
                }
            }

            FileInfo remoteDirectoryInfo;
            hr = connection->GetFileInfo(bstr(remoteSourcePath), &remoteDirectoryInfo);
            if (FAILED(hr)) {
                if (hr != 0x80070002 && hr != 0x80070003 /* Not found */) {
                    qCWarning(lcD3DService) << "Unable to get remote directory info."
                                            << coreConServer->formatError(hr);
                    Sleep(1000);
                    continue;
                }

                // Not found, create remote source path
                hr = connection->MakeDirectory(bstr(remoteSourcePath));
                if (FAILED(hr)) {
                    qCWarning(lcD3DService) << "Unable to create the shader source directory."
                                            << coreConServer->formatError(hr);
                    continue;
                }
            }

            hr = connection->GetFileInfo(bstr(remoteBinaryPath), &remoteDirectoryInfo);
            if (FAILED(hr)) {
                if (hr != 0x80070002 && hr != 0x80070003 /* Not found */) {
                    qCWarning(lcD3DService) << "Unable to get remote directory info."
                                            << coreConServer->formatError(hr);
                    Sleep(1000);
                    continue;
                }

                // Not found, create remote source path
                hr = connection->MakeDirectory(bstr(remoteBinaryPath));
                if (FAILED(hr)) {
                    qCWarning(lcD3DService) << "Unable to create the shader source directory."
                                            << coreConServer->formatError(hr);
                    continue;
                }
            }

            wasDisconnected = false;
        }

        // Update roughly every 30 seconds
        if (round++ % 30 == 0) {
            GetSystemTimeAsFileTime(&controlFileInfo.m_LastWriteTime);
            hr = connection->SetFileInfo(bstr(remoteControlFile), &controlFileInfo);
            round = 1;
            if (FAILED(hr)) {
                qCWarning(lcD3DService) << "Unable to update control file"
                                        << coreConServer->formatError(hr);
                Sleep(1000);
                continue;
            }
        }

        // Ok, ready to check for shaders
        SAFEARRAY *listing;
        hr = connection4->GetDirectoryListing(bstr(remoteSourcePath), &listing);
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << "Unable to get the shader source directory listing"
                                    << coreConServer->formatError(hr);
            wasDisconnected = true;
            Sleep(1000);
            continue;
        }
        QStringList shaderSources;
        if (listing) {
            for (ulong i = 0; i < listing->rgsabound[0].cElements; ++i) {
                LONG indices[] = { i };
                _bstr_t fileName;
                if (SUCCEEDED(SafeArrayGetElement(listing, indices, fileName.GetAddress()))) {
                    // Get the file
                    QString remoteFile = remoteSourcePath + QString::fromWCharArray(fileName);
                    QString localFile = localSourcePath + QString::fromWCharArray(fileName);
                    qCDebug(lcD3DService) << "Found remote shader:" << remoteFile;
                    hr = connection->ReceiveFile(bstr(remoteFile), bstr(localFile), 2);
                    if (FAILED(hr)) {
                        qCWarning(lcD3DService) << "Unable to retrieve the remote shader file:"
                                                << remoteFile << coreConServer->formatError(hr);
                        continue;
                    }
                    // Remove the remote file, push into a list for compilation
                    shaderSources.append(localFile);
                    hr = connection->RemoveFile(bstr(remoteFile));
                    if (FAILED(hr)) {
                        qCWarning(lcD3DService) << "Unable to remove remote shader file:"
                                                << remoteFile << coreConServer->formatError(hr);
                        continue;
                    }
                }
            }
            SafeArrayDestroy(listing);
        }

        foreach (const QString &shaderSource, shaderSources) {
            const QString shaderFileName = QFileInfo(shaderSource).fileName();
            const QString localBinary = localBinaryPath + shaderFileName;
            hr = D3DService::compileShader(shaderSource, localBinary);
            if (FAILED(hr)) {
                qCWarning(lcD3DService) << "Unable to compile shader:" << shaderSource
                                        << coreConServer->formatError(hr);
                continue;
            }

            // All went well, upload the file
            const QString remoteBinary = remoteBinaryPath + shaderFileName;
            hr = connection->SendFile(bstr(localBinary), bstr(remoteBinary), 2, 2, NULL);
            if (FAILED(hr)) {
                qCWarning(lcD3DService) << "Unable to upload binary:"
                                        << remoteBinary << coreConServer->formatError(hr);
                continue;
            }

            qCDebug(lcD3DService) << "Compiled local shader to:" << localBinary
                                  << "and uploaded to" << remoteBinary;
        }

        // Done, take a break.
        Sleep(1000);
    }
}
