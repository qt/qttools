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
#include <QtCore/QDirIterator>
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
using namespace ABI::Windows::Foundation::Collections;

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
    bool isValid() const
    {
        return SUCCEEDED(m_hr);
    }
private:
    HRESULT m_hr;
};

extern int appxAppNames(int deviceIndex, QSet<QString> &apps)
{
    if (deviceIndex) {
        qCWarning(lcD3DService) << "Unsupported device index:" << deviceIndex;
        return 1;
    }

    ComInitializer com;
    if (!com.isValid())
        return 1;

    ComPtr<IPackageManager> packageManager;
    HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Management_Deployment_PackageManager).Get(),
                                    &packageManager);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to instantiate package manager:"
                                << qt_error_string(hr);
        return 1;
    }

    ComPtr<IIterable<Package *>> packageCollection;
    hr = packageManager->FindPackagesByUserSecurityId(NULL, &packageCollection);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Failed to find Appx packages:"
                                << qt_error_string(hr);
        return 1;
    }
    ComPtr<IIterator<Package *>> iterator;
    hr = packageCollection->First(&iterator);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Failed to get package iterator:"
                                << qt_error_string(hr);
        return 1;
    }
    boolean hasCurrent;
    hr = iterator->get_HasCurrent(&hasCurrent);
    while (SUCCEEDED(hr) && hasCurrent) {
        ComPtr<IPackage> package;
        hr = iterator->get_Current(&package);
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << qt_error_string(hr);
            return 1;
        }

#if _MSC_VER >= 1800
        ComPtr<IPackage2> package2;
        hr = package.As(&package2);
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << qt_error_string(hr);
            return 1;
        }

        boolean isDevelopmentMode;
        hr = package2->get_IsDevelopmentMode(&isDevelopmentMode);
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << qt_error_string(hr);
            return 1;
        }
        if (!isDevelopmentMode) {
            hr = iterator->MoveNext(&hasCurrent);
            continue;
        }
#endif // _MSC_VER >= 1800

        ComPtr<IPackageId> id;
        hr = package->get_Id(&id);
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << qt_error_string(hr);
            return 1;
        }

        HString fullName;
        hr = id->get_FullName(fullName.GetAddressOf());
        if (FAILED(hr)) {
            qCWarning(lcD3DService) << qt_error_string(hr);
            return 1;
        }
        apps.insert(QString::fromWCharArray(fullName.GetRawBuffer(NULL)));
        hr = iterator->MoveNext(&hasCurrent);
    }
    return 0;
}

/* This function handles a worker thread servicing an Appx application */
extern int handleAppxDevice(int deviceIndex, const QString &app, const QString &localBase, HANDLE runLock)
{
    if (deviceIndex) {
        qCWarning(lcD3DService) << "Unsupported device index:" << deviceIndex;
        return 1;
    }

    ComInitializer com;
    if (!com.isValid())
        return 1;

    ComPtr<IPackageManager> packageManager;
    HRESULT hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Management_Deployment_PackageManager).Get(),
                            &packageManager);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to instantiate package manager:" << qt_error_string(hr);
        return 1;
    }

    HStringReference packageFullName(reinterpret_cast<LPCWSTR>(app.utf16()));
    ComPtr<IPackage> package;
    hr = packageManager->FindPackageByUserSecurityIdPackageFullName(NULL, packageFullName.Get(), &package);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to query package:" << qt_error_string(hr);
        return 1;
    }
    if (!package) {
        qCWarning(lcD3DService) << "Package is not installed.";
        return 1;
    }
    ComPtr<IPackageId> packageId;
    hr = package->get_Id(&packageId);
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to get package ID:" << qt_error_string(hr);
        return 1;
    }
    HString packageFamilyName;
    hr = packageId->get_FamilyName(packageFamilyName.GetAddressOf());
    if (FAILED(hr)) {
        qCWarning(lcD3DService) << "Unable to get package name:" << qt_error_string(hr);
        return 1;
    }

    const QString localSourcePath = localBase + QStringLiteral("\\source\\");
    const QString localBinaryPath = localBase + QStringLiteral("\\binary\\");

    const QString remoteBase =
            QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
            + QStringLiteral("\\Packages\\")
            + QString::fromWCharArray(packageFamilyName.GetRawBuffer(NULL))
            + QStringLiteral("\\LocalState\\d3dcompiler");
    const QString remoteSourcePath = remoteBase + QStringLiteral("\\source\\");
    const QString remoteBinaryPath = remoteBase + QStringLiteral("\\binary\\");

    bool checkDirectories = true;
    forever {
        // If the run lock is signaled, it's time to quit
        if (WaitForSingleObject(runLock, 0) == WAIT_OBJECT_0)
            return 0;

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

        // Ok, ready to check for shaders
        QDirIterator it(remoteSourcePath);
        while (it.hasNext()) {
            const QString remoteSource = it.next();
            if (!it.fileInfo().isFile())
                continue;
            const QString shaderFileName = it.fileName();
            const QString localSource = localSourcePath + shaderFileName;
            const QString localBinary = localBinaryPath + shaderFileName;

            // Copy remote source to local
            if (QFile::exists(localSource))
                QFile::remove(localSource);
            QFile remoteSourceFile(remoteSource);
            if (!remoteSourceFile.copy(localSource)) {
                qCWarning(lcD3DService) << "Unable to copy shader source:" << remoteSource;
                qCWarning(lcD3DService) << remoteSourceFile.errorString();
                continue;
            }

            // Remove the remote file
            if (!remoteSourceFile.remove()) {
                qCWarning(lcD3DService) << "Unable to remove shader source:" << remoteSource;
                qCWarning(lcD3DService) << remoteSourceFile.errorString();
                continue;
            }

            // Compile shader
            hr = D3DService::compileShader(localSource, localBinary);
            if (FAILED(hr)) {
                qCWarning(lcD3DService) << "Unable to compile shader:" << localSource;
                qCWarning(lcD3DService) << qt_error_string(hr);
                continue;
            }

            // All went well, copy the blob to the device
            const QString remoteBinary = remoteBinaryPath + shaderFileName;
            if (QFile::exists(remoteBinary))
                QFile::remove(remoteBinary);
            QFile localBinaryFile(localBinary);
            if (!localBinaryFile.copy(remoteBinary)) {
                qCWarning(lcD3DService) << "Unable to copy to remote: " << localBinary;
                qCWarning(lcD3DService) << localBinaryFile.errorString();
                continue;
            }

            qCDebug(lcD3DService) << "Compiled local shader to:" << localBinary
                                  << "and uploaded to:" << remoteBinary;
        }

        HANDLE notification = FindFirstChangeNotification(
                    reinterpret_cast<LPCWSTR>(remoteSourcePath.utf16()),
                    FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
        if (!notification) {
            qCCritical(lcD3DService) << "Failed to set up shader directory notification:"
                                     << qt_error_string(GetLastError());
            return 1;
        }

        // Sleep for up to 30 seconds; wake if a new shader appears
        HANDLE waitHandles[] = { notification, runLock };
        DWORD event = WaitForMultipleObjects(2, waitHandles, FALSE, 30000);
        FindCloseChangeNotification(notification);
        // Timeout or directory change; loop and update
        if (event == WAIT_TIMEOUT || event == WAIT_OBJECT_0)
            continue;
        // runLock set; exit
        if (event == WAIT_OBJECT_0 + 1)
            return 0;

        hr = GetLastError();
        // If the app was uninstalled, this is expected
        if (hr == ERROR_INVALID_HANDLE) {
            qCDebug(lcD3DService) << "The wait handle was invalidated; worker exiting.";
            return 1;
        }

        qCWarning(lcD3DService) << "Appx handler wait failed:"
                                << qt_error_string(hr);
        return 1;
    }
}
