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

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>

#include <ShlObj.h>
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

QT_USE_NAMESPACE

#define wchar(str) reinterpret_cast<LPCWSTR>(str.utf16())
#define hStringFromQString(str) HStringReference(reinterpret_cast<const wchar_t *>(str.utf16())).Get()

// Set a break handler for gracefully breaking long-running ops
static bool g_ctrlReceived = false;
static bool g_handleCtrl = false;
static BOOL WINAPI ctrlHandler(DWORD type)
{
    switch (type) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
        g_ctrlReceived = g_handleCtrl;
        return g_handleCtrl;
    case CTRL_BREAK_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
        break;
    }
    return false;
}

class AppxEnginePrivate
{
public:
    Runner *runner;
    bool hasFatalError;

    QString manifest;
    QString packageFullName;
    QString packageFamilyName;
    QString executable;
    qint64 pid;
    HANDLE processHandle;
    DWORD exitCode;

    ComPtr<IPackageManager> packageManager;
    ComPtr<IUriRuntimeClassFactory> uriFactory;
    ComPtr<IApplicationActivationManager> appLauncher;
    ComPtr<IPackageDebugSettings> packageDebug;
};

class XmlStream : public RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>, IStream>
{
public:
    XmlStream(const QString &fileName)
        : m_manifest(fileName)
    {
        m_manifest.open(QFile::ReadOnly);
    }

    ~XmlStream()
    {
    }

    HRESULT __stdcall Read(void *data, ULONG count, ULONG *bytesRead)
    {
        *bytesRead = m_manifest.read(reinterpret_cast<char *>(data), count);
        return S_OK;
    }

    HRESULT __stdcall Seek(LARGE_INTEGER pos, DWORD start, ULARGE_INTEGER *newPos)
    {
        switch (start) {
        default:
        case STREAM_SEEK_SET:
            /* pos.QuadPart += 0; */
            break;
        case STREAM_SEEK_CUR:
            pos.QuadPart += m_manifest.pos();
            break;
        case STREAM_SEEK_END:
            pos.QuadPart += m_manifest.size();
            break;
        }
        if (!m_manifest.seek(pos.QuadPart))
            return STG_E_INVALIDPOINTER;
        if (newPos) {
            ULARGE_INTEGER newPosInt = { m_manifest.pos() };
            *newPos = newPosInt;
        }
        return S_OK;
    }

    HRESULT __stdcall Stat(STATSTG *stats, DWORD flags)
    {
        QFileInfo info(m_manifest);
        // ms to 100-ns units
        ULARGE_INTEGER lastModifiedInt = { info.lastModified().toMSecsSinceEpoch() * 10000 };
        ULARGE_INTEGER createdInt = { info.created().toMSecsSinceEpoch() * 10000 };
        ULARGE_INTEGER lastReadInt = { info.lastRead().toMSecsSinceEpoch() * 10000 };
        STATSTG newStats = {
            flags == STATFLAG_NONAME ? NULL : reinterpret_cast<LPWSTR>(m_manifest.fileName().data()),
            STGTY_STREAM, { m_manifest.size() },
            { lastModifiedInt.u.LowPart, lastModifiedInt.u.HighPart },
            { createdInt.u.LowPart, createdInt.u.HighPart },
            { lastReadInt.u.LowPart, lastReadInt.u.HighPart },
            0, 0, CLSID_NULL, 0, 0
        };
        *stats = newStats;
        return S_OK;
    }

    // Unimplemented methods
    HRESULT __stdcall Write(const void *, ULONG, ULONG *) { return E_NOTIMPL; }
    HRESULT __stdcall SetSize(ULARGE_INTEGER) { return E_NOTIMPL; }
    HRESULT __stdcall CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) { return E_NOTIMPL; }
    HRESULT __stdcall Commit(DWORD) { return E_NOTIMPL; }
    HRESULT __stdcall Revert() { return E_NOTIMPL; }
    HRESULT __stdcall LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
    HRESULT __stdcall UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) { return E_NOTIMPL; }
    HRESULT __stdcall Clone(IStream **) { return E_NOTIMPL; }

private:
    QFile m_manifest;
};

static bool getManifestFile(const QString &fileName, QString *manifest = 0)
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

bool AppxEngine::canHandle(Runner *runner)
{
    return getManifestFile(runner->app());
}

RunnerEngine *AppxEngine::create(Runner *runner)
{
    AppxEngine *engine = new AppxEngine(runner);
    if (engine->d_ptr->hasFatalError) {
        delete engine;
        return 0;
    }
    return engine;
}

QStringList AppxEngine::deviceNames()
{
    qCDebug(lcWinRtRunner) << __FUNCTION__;
    return QStringList(QStringLiteral("local"));
}

AppxEngine::AppxEngine(Runner *runner) : d_ptr(new AppxEnginePrivate)
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
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to initialize COM. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Management_Deployment_PackageManager).Get(),
                            &d->packageManager);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to instantiate package manager. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Foundation_Uri).Get(),
                                IID_PPV_ARGS(&d->uriFactory));
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to instantiate URI factory. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IApplicationActivationManager, &d->appLauncher);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to instantiate application activation manager. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    hr = CoCreateInstance(CLSID_PackageDebugSettings, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPackageDebugSettings, &d->packageDebug);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to instantiate package debug settings. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    ComPtr<IAppxFactory> packageFactory;
    hr = CoCreateInstance(CLSID_AppxFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IAppxFactory, &packageFactory);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to instantiate package factory. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    ComPtr<IStream> manifestStream = Make<XmlStream>(d->manifest);
    ComPtr<IAppxManifestReader> manifestReader;
    hr = packageFactory->CreateManifestReader(manifestStream.Get(), &manifestReader);
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
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Unable to obtain the package ID from the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    LPWSTR packageFullName;
    hr = packageId->GetPackageFullName(&packageFullName);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Unable to obtain the package full name from the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }
    d->packageFullName = QString::fromWCharArray(packageFullName);
    CoTaskMemFree(packageFullName);

    LPWSTR packageFamilyName;
    hr = packageId->GetPackageFamilyName(&packageFamilyName);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Unable to obtain the package full family name from the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }
    d->packageFamilyName = QString::fromWCharArray(packageFamilyName);
    CoTaskMemFree(packageFamilyName);

    ComPtr<IAppxManifestApplicationsEnumerator> applications;
    hr = manifestReader->GetApplications(&applications);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to get a list of applications from the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    BOOL hasCurrent;
    hr = applications->GetHasCurrent(&hasCurrent);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to iterate over applications in the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    // For now, we are only interested in the first application
    ComPtr<IAppxManifestApplication> application;
    hr = applications->GetCurrent(&application);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to access the first application in the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }

    LPWSTR executable;
    application->GetStringValue(L"Executable", &executable);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to retrieve the application executable from the manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        d->hasFatalError = true;
        return;
    }
    d->executable = QFileInfo(d->manifest).absoluteDir()
            .absoluteFilePath(QString::fromWCharArray(executable));
    CoTaskMemFree(executable);

    // Set a break handler for gracefully exiting from long-running operations
    SetConsoleCtrlHandler(&ctrlHandler, true);
}

AppxEngine::~AppxEngine()
{
    Q_D(const AppxEngine);
    CloseHandle(d->processHandle);
}

bool AppxEngine::install(bool removeFirst)
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    ComPtr<IPackage> packageInformation;
    HRESULT hr = d->packageManager->FindPackageByUserSecurityIdPackageFullName(
                NULL, hStringFromQString(d->packageFullName), &packageInformation);
    if (SUCCEEDED(hr) && packageInformation) {
        qCWarning(lcWinRtRunner) << "Package already installed.";
        if (removeFirst)
            remove();
        else
            return true;
    }

    const QString appPath = QDir::toNativeSeparators(QFileInfo(d->manifest).absoluteFilePath());
    ComPtr<IUriRuntimeClass> uri;
    hr = d->uriFactory->CreateUri(hStringFromQString(appPath), &uri);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Unable to create a URI for the package manifest. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';

        return false;
    }

    ComPtr<IAsyncOperationWithProgress<DeploymentResult *, DeploymentProgress>> deploymentOperation;
    hr = d->packageManager->RegisterPackageAsync(uri.Get(), 0, DeploymentOptions_DevelopmentMode, &deploymentOperation);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Unable to start package registration. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        return false;
    }

    ComPtr<IDeploymentResult> results;
    while ((hr = deploymentOperation->GetResults(&results)) == E_ILLEGAL_METHOD_CALL)
        Sleep(1);

    HRESULT errorCode;
    hr = results->get_ExtendedErrorCode(&errorCode);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Unable to retrieve package registration results. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        return false;
    }

    if (FAILED(errorCode)) {
        HSTRING errorText;
        if (SUCCEEDED(results->get_ErrorText(&errorText))) {
            qCWarning(lcWinRtRunner) << "Unable to register package:"
                                     << QString::fromWCharArray(WindowsGetStringRawBuffer(errorText, 0));
        }
        if (HRESULT_CODE(errorCode) == ERROR_INSTALL_POLICY_FAILURE) {
            // The user's license has expired. Give them the opportunity to renew it.
            FILETIME expiration;
            hr = AcquireDeveloperLicense(GetForegroundWindow(), &expiration);
            if (FAILED(hr)) {
                qCWarning(lcWinRtRunner) << "Unable to renew developer license:"
                                         << qt_error_string(hr);
            }
            if (SUCCEEDED(hr))
                return install(false);
        }
        return false;
    }

    return SUCCEEDED(hr);
}

bool AppxEngine::remove()
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    ComPtr<IAsyncOperationWithProgress<DeploymentResult *, DeploymentProgress>> deploymentOperation;
    HRESULT hr = d->packageManager->RemovePackageAsync(hStringFromQString(d->packageFullName), &deploymentOperation);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner) << "Unable to start package removal:" << QDir::toNativeSeparators(d->manifest);
        return false;
    }

    ComPtr<IDeploymentResult> results;
    while ((hr = deploymentOperation.Get()->GetResults(&results)) == E_ILLEGAL_METHOD_CALL)
        Sleep(1);

    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner) << "Unable to remove package:" << QDir::toNativeSeparators(d->manifest);
        return false;
    }

    return SUCCEEDED(hr);
}

bool AppxEngine::start()
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    const QString launchArguments = d->runner->arguments().join(QLatin1Char(' '));
    DWORD pid;
    const QString activationId = d->packageFamilyName + QStringLiteral("!App");
    HRESULT hr = d->appLauncher->ActivateApplication(wchar(activationId),
                                                     wchar(launchArguments), AO_NONE, &pid);
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to activate application. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        return false;
    }
    d->pid = qint64(pid);
    CloseHandle(d->processHandle);
    d->processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, true, pid);

    return true;
}

bool AppxEngine::suspend()
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    HRESULT hr = d->packageDebug->Suspend(wchar(d->packageFullName));
    if (FAILED(hr)) {
        qCWarning(lcWinRtRunner).nospace() << "Failed to suspend application. (0x"
                                           << QByteArray::number(hr, 16).constData()
                                           << ' ' << qt_error_string(hr) << ')';
        return false;
    }

    return true;
}

bool AppxEngine::waitForFinished(int secs)
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    g_handleCtrl = true;
    int time = 0;
    forever {
        PACKAGE_EXECUTION_STATE state;
        HRESULT hr = d->packageDebug->GetPackageExecutionState(wchar(d->packageFullName), &state);
        if (FAILED(hr)) {
            qCWarning(lcWinRtRunner).nospace() << "Failed to get package execution state. (0x"
                                               << QByteArray::number(hr, 16).constData()
                                               << ' ' << qt_error_string(hr) << ')';
            return false;
        }
        qCDebug(lcWinRtRunner) << "Current execution state:" << state;
        if (state == PES_TERMINATED)
            break;

        ++time;
        if ((secs && time > secs) || g_ctrlReceived) {
            g_handleCtrl = false;
            return false;
        }

        Sleep(1000); // Wait one second between checks
        qCDebug(lcWinRtRunner) << "Waiting for app to quit - msecs to go:" << secs - time;
    }
    g_handleCtrl = false;

    if (!GetExitCodeProcess(d->processHandle, &d->exitCode))
        d->exitCode = UINT_MAX;

    return true;
}

bool AppxEngine::stop()
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    // ### We won't have a process handle if we didn't start the app. We can look it up
    // using a process snapshot, or by calling start if we know the process is already running.
    // For now, simply continue normally, but don't fetch the exit code.
    if (!d->processHandle)
        qCDebug(lcWinRtRunner) << "No handle to the process; the exit code won't be available.";

    if (d->processHandle && !GetExitCodeProcess(d->processHandle, &d->exitCode)) {
        d->exitCode = UINT_MAX;
        qCWarning(lcWinRtRunner).nospace() << "Failed to obtain process exit code.";
        qCDebug(lcWinRtRunner, "GetLastError: 0x%x", GetLastError());
        return false;
    }

    if (!d->processHandle || d->exitCode == STILL_ACTIVE) {
        HRESULT hr = d->packageDebug->TerminateAllProcesses(wchar(d->packageFullName));
        if (FAILED(hr)) {
            qCWarning(lcWinRtRunner).nospace() << "Failed to terminate package process. (0x"
                                               << QByteArray::number(hr, 16).constData()
                                               << ' ' << qt_error_string(hr) << ')';
            return false;
        }

        if (d->processHandle && !GetExitCodeProcess(d->processHandle, &d->exitCode))
            d->exitCode = UINT_MAX;
    }

    return true;
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

QString AppxEngine::devicePath(const QString &relativePath) const
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    // Return a path safe for passing to the application
    QDir localAppDataPath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    const QString path = localAppDataPath.absoluteFilePath(
                QStringLiteral("Packages/") + d->packageFamilyName
                + QStringLiteral("/LocalState/") + relativePath);
    return QDir::toNativeSeparators(path);
}

bool AppxEngine::sendFile(const QString &localFile, const QString &deviceFile)
{
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    // Both files are local, just use QFile
    QFile source(localFile);

    // Remove the destination, or copy will fail
    if (QFileInfo(source) != QFileInfo(deviceFile))
        QFile::remove(deviceFile);

    bool result = source.copy(deviceFile);
    if (!result)
        qCWarning(lcWinRtRunner) << "Unable to sendFile:" << source.errorString();

    return result;
}

bool AppxEngine::receiveFile(const QString &deviceFile, const QString &localFile)
{
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    // Both files are local, so just reverse the sendFile arguments
    return sendFile(deviceFile, localFile);
}
