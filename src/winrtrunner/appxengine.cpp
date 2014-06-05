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

#include "appxengine.h"
#include "appxengine_p.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>

#include <ShlObj.h>
#include <Shlwapi.h>
#include <wsdevlicensing.h>
#include <AppxPackaging.h>
#include <wrl.h>
#include <windows.applicationmodel.h>
#include <windows.management.deployment.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Management::Deployment;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::System;

QT_USE_NAMESPACE

bool AppxEngine::getManifestFile(const QString &fileName, QString *manifest)
{
    if (!QFile::exists(fileName)) {
        qCWarning(lcWinRtRunner) << fileName << "does not exist.";
        return false;
    }

    // If it looks like an appx manifest, we're done
    if (fileName.endsWith(QStringLiteral("AppxManifest.xml"))) {

        if (manifest)
            *manifest = fileName;
        return true;
    }

    // If it looks like an executable, check that manifest is next to it
    if (fileName.endsWith(QStringLiteral(".exe"))) {
        QDir appDir = QFileInfo(fileName).absoluteDir();
        QString manifestFileName = appDir.absoluteFilePath(QStringLiteral("AppxManifest.xml"));
        if (!QFile::exists(manifestFileName)) {
            qCWarning(lcWinRtRunner) << manifestFileName << "does not exist.";
            return false;
        }

        if (manifest)
            *manifest = manifestFileName;
        return true;
    }

    // TODO: handle already-built package as well

    qCWarning(lcWinRtRunner) << "Appx: unable to determine manifest for" << fileName << ".";
    return false;
}

#define CHECK_RESULT(errorMessage, action)\
    do {\
        if (FAILED(hr)) {\
            qCWarning(lcWinRtRunner).nospace() << errorMessage " (0x"\
                                               << QByteArray::number(hr, 16).constData()\
                                               << ' ' << qt_error_string(hr) << ')';\
            action;\
        }\
    } while (false)

#define CHECK_RESULT_FATAL(errorMessage, action)\
    do {CHECK_RESULT(errorMessage, d->hasFatalError = true; action;);} while (false)

static ProcessorArchitecture toProcessorArchitecture(APPX_PACKAGE_ARCHITECTURE appxArch)
{
    switch (appxArch) {
    case APPX_PACKAGE_ARCHITECTURE_X86:
        return ProcessorArchitecture_X86;
    case APPX_PACKAGE_ARCHITECTURE_ARM:
        return ProcessorArchitecture_Arm;
    case APPX_PACKAGE_ARCHITECTURE_X64:
        return ProcessorArchitecture_X64;
    case APPX_PACKAGE_ARCHITECTURE_NEUTRAL:
        // fall-through intended
    default:
        return ProcessorArchitecture_Neutral;
    }
}

AppxEngine::AppxEngine(Runner *runner, AppxEnginePrivate *dd)
    : d_ptr(dd)
{
    Q_D(AppxEngine);
    d->runner = runner;
    d->hasFatalError = false;
    d->processHandle = NULL;
    d->pid = -1;
    d->exitCode = UINT_MAX;

    if (!getManifestFile(runner->app(), &d->manifest)) {
        qCWarning(lcWinRtRunner) << "Unable to determine manifest file from" << runner->app();
        d->hasFatalError = true;
        return;
    }

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    CHECK_RESULT_FATAL("Failed to initialize COM.", return);

    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Foundation_Uri).Get(),
                                IID_PPV_ARGS(&d->uriFactory));
    CHECK_RESULT_FATAL("Failed to instantiate URI factory.", return);

    hr = CoCreateInstance(CLSID_AppxFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IAppxFactory, &d->packageFactory);
    CHECK_RESULT_FATAL("Failed to instantiate package factory.", return);

    ComPtr<IStream> manifestStream;
    hr = SHCreateStreamOnFile(wchar(d->manifest), STGM_READ, &manifestStream);
    CHECK_RESULT_FATAL("Failed to open manifest stream.", return);

    ComPtr<IAppxManifestReader> manifestReader;
    hr = d->packageFactory->CreateManifestReader(manifestStream.Get(), &manifestReader);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to instantiate manifest reader. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        // ### TODO: read detailed error from event log directly
        if (hr == APPX_E_INVALID_MANIFEST) {
            qCWarning(lcWinRtRunner) << "More information on the error can "
                                      "be found in the event log under "
                                      "Microsoft\\Windows\\AppxPackagingOM";
        }
        d->hasFatalError = true;
        return;
    }

    ComPtr<IAppxManifestPackageId> packageId;
    hr = manifestReader->GetPackageId(&packageId);
    CHECK_RESULT_FATAL("Unable to obtain the package ID from the manifest.", return);

    APPX_PACKAGE_ARCHITECTURE arch;
    hr = packageId->GetArchitecture(&arch);
    CHECK_RESULT_FATAL("Failed to retrieve the app's architecture.", return);
    d->packageArchitecture = toProcessorArchitecture(arch);

    LPWSTR packageFullName;
    hr = packageId->GetPackageFullName(&packageFullName);
    CHECK_RESULT_FATAL("Unable to obtain the package full name from the manifest.", return);
    d->packageFullName = QString::fromWCharArray(packageFullName);
    CoTaskMemFree(packageFullName);

    LPWSTR packageFamilyName;
    hr = packageId->GetPackageFamilyName(&packageFamilyName);
    CHECK_RESULT_FATAL("Unable to obtain the package full family name from the manifest.", return);
    d->packageFamilyName = QString::fromWCharArray(packageFamilyName);
    CoTaskMemFree(packageFamilyName);

    ComPtr<IAppxManifestApplicationsEnumerator> applications;
    hr = manifestReader->GetApplications(&applications);
    CHECK_RESULT_FATAL("Failed to get a list of applications from the manifest.", return);

    BOOL hasCurrent;
    hr = applications->GetHasCurrent(&hasCurrent);
    CHECK_RESULT_FATAL("Failed to iterate over applications in the manifest.", return);

    // For now, we are only interested in the first application
    ComPtr<IAppxManifestApplication> application;
    hr = applications->GetCurrent(&application);
    CHECK_RESULT_FATAL("Failed to access the first application in the manifest.", return);

    LPWSTR executable;
    application->GetStringValue(L"Executable", &executable);
    CHECK_RESULT_FATAL("Failed to retrieve the application executable from the manifest.", return);
    d->executable = QFileInfo(d->manifest).absoluteDir()
            .absoluteFilePath(QString::fromWCharArray(executable));
    CoTaskMemFree(executable);

    ComPtr<IAppxManifestPackageDependenciesEnumerator> dependencies;
    hr = manifestReader->GetPackageDependencies(&dependencies);
    CHECK_RESULT_FATAL("Failed to retrieve the package dependencies from the manifest.", return);

    hr = dependencies->GetHasCurrent(&hasCurrent);
    CHECK_RESULT_FATAL("Failed to iterate over dependencies in the manifest.", return);
    while (SUCCEEDED(hr) && hasCurrent) {
        ComPtr<IAppxManifestPackageDependency> dependency;
        hr = dependencies->GetCurrent(&dependency);
        CHECK_RESULT_FATAL("Failed to access dependency in the manifest.", return);

        LPWSTR name;
        hr = dependency->GetName(&name);
        CHECK_RESULT_FATAL("Failed to access dependency name.", return);
        d->dependencies.insert(QString::fromWCharArray(name));
        CoTaskMemFree(name);
        hr = dependencies->MoveNext(&hasCurrent);
    }
}

AppxEngine::~AppxEngine()
{
    Q_D(const AppxEngine);
    CloseHandle(d->processHandle);
}

qint64 AppxEngine::pid() const
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    return d->pid;
}

int AppxEngine::exitCode() const
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    return d->exitCode == UINT_MAX ? -1 : HRESULT_CODE(d->exitCode);
}

QString AppxEngine::executable() const
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    return d->executable;
}

bool AppxEngine::installDependencies()
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    QSet<QString> toInstall;
    foreach (const QString &dependencyName, d->dependencies) {
        toInstall.insert(dependencyName);
        qCDebug(lcWinRtRunner).nospace()
            << "dependency to be installed: " << dependencyName;
    }

    if (toInstall.isEmpty())
        return true;

    const QString extensionSdkDir = extensionSdkPath();
    if (!QFile::exists(extensionSdkDir)) {
        qCWarning(lcWinRtRunner).nospace()
                << QString(QStringLiteral("The directory '%1' does not exist.")).arg(
                       QDir::toNativeSeparators(extensionSdkDir));
        return false;
    }
    qCDebug(lcWinRtRunner).nospace()
        << "looking for dependency packages in " << extensionSdkDir;
    QDirIterator dit(extensionSdkDir, QStringList() << QStringLiteral("*.appx"),
                     QDir::Files,
                     QDirIterator::Subdirectories);
    while (dit.hasNext()) {
        dit.next();

        HRESULT hr;
        ComPtr<IStream> inputStream;
        hr = SHCreateStreamOnFileEx(wchar(dit.filePath()),
                                    STGM_READ | STGM_SHARE_EXCLUSIVE,
                                    0, FALSE, NULL, &inputStream);
        CHECK_RESULT("Failed to create input stream for package in ExtensionSdkDir.", continue);

        ComPtr<IAppxPackageReader> packageReader;
        hr = d->packageFactory->CreatePackageReader(inputStream.Get(), &packageReader);
        CHECK_RESULT("Failed to create package reader for package in ExtensionSdkDir.", continue);

        ComPtr<IAppxManifestReader> manifestReader;
        hr = packageReader->GetManifest(&manifestReader);
        CHECK_RESULT("Failed to create manifest reader for package in ExtensionSdkDir.", continue);

        ComPtr<IAppxManifestPackageId> packageId;
        hr = manifestReader->GetPackageId(&packageId);
        CHECK_RESULT("Failed to retrieve package id for package in ExtensionSdkDir.", continue);

        LPWSTR sz;
        hr = packageId->GetName(&sz);
        CHECK_RESULT("Failed to retrieve name from package in ExtensionSdkDir.", continue);
        const QString name = QString::fromWCharArray(sz);
        CoTaskMemFree(sz);

        if (!toInstall.contains(name))
            continue;

        APPX_PACKAGE_ARCHITECTURE arch;
        hr = packageId->GetArchitecture(&arch);
        CHECK_RESULT("Failed to retrieve architecture from package in ExtensionSdkDir.", continue);
        if (d->packageArchitecture != arch)
            continue;

        qCDebug(lcWinRtRunner).nospace()
            << "installing dependency " << name << " from " << dit.filePath();
        if (!installPackage(manifestReader.Get(), dit.filePath())) {
            qCWarning(lcWinRtRunner) << "Failed to install package:" << name;
            return false;
        }
    }

    return true;
}
