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

#include <windows.applicationmodel.core.h>
#include <windows.management.deployment.h>
#include <windows.storage.h>
#include <comdef.h>
#include <wrl.h>
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Management::Deployment;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::Storage;

struct ComInitializer
{
    ComInitializer()
    {
        m_hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(m_hr))
            qCWarning(lcD3DService) << "Unable to initialize COM.";
    }
    ~ComInitializer()
    {
        if (isValid())
            CoUninitialize();
    }
    bool isValid()
    {
        return SUCCEEDED(m_hr);
    }
private:
    HRESULT m_hr;
};

/* This function handles a worker thread servicing an Appx application */
extern void handleAppxDevice(int deviceIndex, const QString &app, const QString &localBase)
{
    if (deviceIndex) {
        qCWarning(lcD3DService) << "Unsupported device index:" << deviceIndex;
        return;
    }

    ComInitializer com;
    if (!com.isValid())
        return;

    ComPtr<IPackageManager> packageManager;
    HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Management_Deployment_PackageManager).Get(),
                            &packageManager);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to instantiate package manager. HRESULT: 0x" << QByteArray::number(hr, 16).constData();
        return;
    }

    HStringReference packageFullName(reinterpret_cast<LPCWSTR>(app.utf16()));
    ComPtr<IPackage> package;
    hr = packageManager->FindPackageByUserSecurityIdPackageFullName(NULL, packageFullName.Get(), &package);
    if (FAILED(hr)) {
        qCWarning(lcD3DService).nospace() << "Unable to query package. HRESULT: 0x"
                                          << QByteArray::number(hr, 16).constData();
        return;
    }
    if (!package) {
        qCWarning(lcD3DService) << "Package is not installed.";
        return;
    }
    ComPtr<IPackageId> packageId;
    hr = package->get_Id(&packageId);
    if (FAILED(hr)) {
        qCWarning(lcD3DService).nospace() << "Unable to get package ID. HRESULT: 0x"
                                          << QByteArray::number(hr, 16).constData();
        return;
    }
    HSTRING packageFamilyName;
    hr = packageId->get_FamilyName(&packageFamilyName);
    if (FAILED(hr)) {
        qCWarning(lcD3DService).nospace() << "Unable to get package name. HRESULT: 0x"
                                          << QByteArray::number(hr, 16).constData();
        return;
    }

    const QString localSourcePath = localBase + QStringLiteral("\\source\\");
    const QString localBinaryPath = localBase + QStringLiteral("\\binary\\");

    const QString remoteBase =
            QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
            + QStringLiteral("\\Packages\\")
            + QString::fromWCharArray(WindowsGetStringRawBuffer(packageFamilyName, Q_NULLPTR))
            + QStringLiteral("\\LocalState\\d3dcompiler");
    const QString remoteControlFile = remoteBase + QStringLiteral("\\control");
    const QString remoteSourcePath = remoteBase + QStringLiteral("\\source\\");
    const QString remoteBinaryPath = remoteBase + QStringLiteral("\\binary\\");

    D3DService::reportStatus(SERVICE_RUNNING, NO_ERROR, 0);

    int round = 0;
    bool checkDirectories = true;
    forever {
        // Run certain setup steps once per connection
        if (checkDirectories) {
            // Check remote directory
            QDir dir(remoteBase);
            if (!dir.exists()) {
                dir.cdUp();
                if (!dir.mkpath(QStringLiteral("d3dcompiler"))) {
                    qCWarning(lcD3DService) << "Could not create d3dcompiler directory.";
                    Sleep(1000);
                    continue;
                }
                dir.cd(QStringLiteral("d3dcompiler"));
            }

            // Check if control file exists
            if (!QFile::exists(remoteControlFile)) {
                QFile file(remoteControlFile);
                if (!file.open(QFile::WriteOnly)) {
                    qCWarning(lcD3DService) << "Could not create control file:" << file.errorString();
                    Sleep(1000);
                    continue;
                }
                file.write("Qt D3D compilation service");
            }

            if (!QFile::exists(remoteSourcePath)) {
                if (!dir.mkpath(QStringLiteral("source"))) {
                    qCWarning(lcD3DService) << "Could not create source directory.";
                    Sleep(1000);
                    continue;
                }
            }

            if (!QFile::exists(remoteBinaryPath)) {
                if (!dir.mkpath(QStringLiteral("binary"))) {
                    qCWarning(lcD3DService) << "Could not create binary directory.";
                    Sleep(1000);
                    continue;
                }
            }

            checkDirectories = false;
        }

        // Update roughly every 30 seconds
        if (round++ % 30 == 0) {
            QFile file(remoteControlFile);
            if (!file.open(QFile::WriteOnly)) {
                qCWarning(lcD3DService) << "Could not create control file.";
                Sleep(1000);
                continue;
            }
            file.write("Qt D3D compilation service");
        }

        // Ok, ready to check for shaders
        QDir remoteSourceDir(remoteSourcePath);
        const QStringList shaderSources = remoteSourceDir.entryList(QDir::Files);
        foreach (const QString &shaderSource, shaderSources) {
            const QString remoteSource = remoteSourceDir.absoluteFilePath(shaderSource);
            const QString shaderFileName = QFileInfo(remoteSource).fileName();
            const QString localSource = localSourcePath + shaderFileName;
            const QString localBinary = localBinaryPath + shaderFileName;

            // Copy remote source to local
            if (!QFile::copy(remoteSource, localSource)) {
                qCWarning(lcD3DService) << "Unable to copy shader source:" << remoteSource;
                continue;
            }

            // Remove the remote file
            if (!QFile::remove(remoteSource)) {
                qCWarning(lcD3DService) << "Unable to remove shader source:" << remoteSource;
                continue;
            }

            // Compile shader
            hr = D3DService::compileShader(localSource, localBinary);
            if (FAILED(hr)) {
                qCWarning(lcD3DService) << "Unable to compile shader:" << shaderSource;
                qCWarning(lcD3DService) << qt_error_string(hr);
                continue;
            }

            // All went well, copy the blob to the device
            const QString remoteBinary = remoteBinaryPath + shaderFileName;
            if (!QFile::copy(localBinary, remoteBinary)) {
                qCWarning(lcD3DService) << "Unable to copy to remote: " << localBinary;
                continue;
            }

            qCDebug(lcD3DService) << "Compiled local shader to:" << localBinary
                                  << "and uploaded to:" << remoteBinary;
        }

        // Done, take a break.
        Sleep(1000);
    }
}
