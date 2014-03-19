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

#define wchar(str) reinterpret_cast<LPCWSTR>(str.utf16())
#define hStringFromQString(str) HStringReference(reinterpret_cast<const wchar_t *>(str.utf16())).Get()
#define QStringFromHString(hstr) QString::fromWCharArray(WindowsGetStringRawBuffer(hstr, Q_NULLPTR))

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

QString sidForPackage(const QString &packageFamilyName)
{
    QString sid;
    HKEY regKey;
    LONG result = RegOpenKeyEx(
                HKEY_CLASSES_ROOT,
                L"Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppContainer\\Mappings",
                0, KEY_READ, &regKey);
    if (result != ERROR_SUCCESS) {
        qCWarning(lcWinRtRunner) << "Unable to open registry key:" << qt_error_string(result);
        return sid;
    }

    DWORD index = 0;
    wchar_t subKey[MAX_PATH];
    forever {
        result = RegEnumKey(regKey, index++, subKey, MAX_PATH);
        if (result != ERROR_SUCCESS)
            break;
        wchar_t moniker[MAX_PATH];
        DWORD monikerSize = MAX_PATH;
        result = RegGetValue(regKey, subKey, L"Moniker", RRF_RT_REG_SZ, NULL, moniker, &monikerSize);
        if (result != ERROR_SUCCESS)
            continue;
        if (lstrcmp(moniker, reinterpret_cast<LPCWSTR>(packageFamilyName.utf16())) == 0) {
            sid = QString::fromWCharArray(subKey);
            break;
        }
    }
    RegCloseKey(regKey);
    return sid;
}

class OutputDebugMonitor
{
public:
    OutputDebugMonitor()
        : runLock(CreateEvent(NULL, FALSE, FALSE, NULL)), thread(0)
    {
    }
    ~OutputDebugMonitor()
    {
        if (runLock) {
            SetEvent(runLock);
            CloseHandle(runLock);
        }
        if (thread) {
            WaitForSingleObject(thread, INFINITE);
            CloseHandle(thread);
        }
    }
    void start(const QString &packageFamilyName)
    {
        if (thread) {
            qCWarning(lcWinRtRunner) << "OutputDebugMonitor is already running.";
            return;
        }

        package = packageFamilyName;

        thread = CreateThread(NULL, 0, &monitor, this, NULL, NULL);
        if (!thread) {
            qCWarning(lcWinRtRunner) << "Unable to create thread for app debugging:"
                                     << qt_error_string(GetLastError());
            return;
        }

        return;
    }
private:
    static DWORD __stdcall monitor(LPVOID param)
    {
        OutputDebugMonitor *that = static_cast<OutputDebugMonitor *>(param);

        const QString handleBase = QStringLiteral("Local\\AppContainerNamedObjects\\")
                + sidForPackage(that->package);
        const QString eventName = handleBase + QStringLiteral("\\qdebug-event");
        const QString shmemName = handleBase + QStringLiteral("\\qdebug-shmem");

        HANDLE event = CreateEvent(NULL, FALSE, FALSE, reinterpret_cast<LPCWSTR>(eventName.utf16()));
        if (!event) {
            qCWarning(lcWinRtRunner) << "Unable to open shared event for app debugging:"
                                     << qt_error_string(GetLastError());
            return 1;
        }

        HANDLE shmem = 0;
        DWORD ret = 0;
        forever {
            HANDLE handles[] = { that->runLock, event };
            DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

            // runLock set; exit thread
            if (result == WAIT_OBJECT_0)
                break;

            // debug event set; print message
            if (result == WAIT_OBJECT_0 + 1) {
                if (!shmem) {
                    shmem = OpenFileMapping(GENERIC_READ, FALSE,
                                            reinterpret_cast<LPCWSTR>(shmemName.utf16()));
                    if (!shmem) {
                        qCWarning(lcWinRtRunner) << "Unable to open shared memory for app debugging:"
                                                 << qt_error_string(GetLastError());
                        ret = 1;
                        break;
                    }
                }

                const quint32 *data = reinterpret_cast<const quint32 *>(
                            MapViewOfFile(shmem, FILE_MAP_READ, 0, 0, 4096));
                QtMsgType messageType = static_cast<QtMsgType>(data[0]);
                QString message = QString::fromWCharArray(
                            reinterpret_cast<const wchar_t *>(data + 1));
                UnmapViewOfFile(data);
                switch (messageType) {
                default:
                case QtDebugMsg:
                    qCDebug(lcWinRtRunnerApp, qPrintable(message));
                    break;
                case QtWarningMsg:
                    qCWarning(lcWinRtRunnerApp, qPrintable(message));
                    break;
                case QtCriticalMsg:
                case QtFatalMsg:
                    qCCritical(lcWinRtRunnerApp, qPrintable(message));
                    break;
                }
                continue;
            }

            // An error occurred; exit thread
            qCWarning(lcWinRtRunner) << "Debug output monitor error:"
                                     << qt_error_string(GetLastError());
            ret = 1;
            break;
        }
        if (shmem)
            CloseHandle(shmem);
        if (event)
            CloseHandle(event);
        return ret;
    }
    HANDLE runLock;
    HANDLE thread;
    QString package;
};
Q_GLOBAL_STATIC(OutputDebugMonitor, debugMonitor)

class AppxEnginePrivate
{
public:
    Runner *runner;
    bool hasFatalError;

    QString manifest;
    QString packageFullName;
    QString packageFamilyName;
    ProcessorArchitecture packageArchitecture;
    QString executable;
    qint64 pid;
    HANDLE processHandle;
    DWORD exitCode;
    QSet<QString> dependencies;
    QSet<QString> installedPackages;

    ComPtr<IPackageManager> packageManager;
    ComPtr<IUriRuntimeClassFactory> uriFactory;
    ComPtr<IApplicationActivationManager> appLauncher;
    ComPtr<IAppxFactory> packageFactory;
    ComPtr<IPackageDebugSettings> packageDebug;

    void retrieveInstalledPackages();
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
    CHECK_RESULT_FATAL("Failed to initialize COM.", return);

    hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Management_Deployment_PackageManager).Get(),
                            &d->packageManager);
    CHECK_RESULT_FATAL("Failed to instantiate package manager.", return);

    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Foundation_Uri).Get(),
                                IID_PPV_ARGS(&d->uriFactory));
    CHECK_RESULT_FATAL("Failed to instantiate URI factory.", return);

    hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IApplicationActivationManager, &d->appLauncher);
    CHECK_RESULT_FATAL("Failed to instantiate application activation manager.", return);

    hr = CoCreateInstance(CLSID_PackageDebugSettings, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IPackageDebugSettings, &d->packageDebug);
    CHECK_RESULT_FATAL("Failed to instantiate package debug settings.", return);

    hr = CoCreateInstance(CLSID_AppxFactory, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IAppxFactory, &d->packageFactory);
    CHECK_RESULT_FATAL("Failed to instantiate package factory.", return);

    ComPtr<IStream> manifestStream = Make<XmlStream>(d->manifest);
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

    d->retrieveInstalledPackages();

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

    // Set a break handler for gracefully exiting from long-running operations
    SetConsoleCtrlHandler(&ctrlHandler, true);
}

AppxEngine::~AppxEngine()
{
    Q_D(const AppxEngine);
    CloseHandle(d->processHandle);
}

bool AppxEngine::installDependencies()
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    QSet<QString> toInstall;
    foreach (const QString &dependencyName, d->dependencies) {
        if (d->installedPackages.contains(dependencyName))
            continue;
        toInstall.insert(dependencyName);
        qCDebug(lcWinRtRunner).nospace()
            << "dependency to be installed: " << dependencyName;
    }

    if (toInstall.isEmpty())
        return true;

    const QByteArray extensionSdkDirRaw = qgetenv("ExtensionSdkDir");
    if (extensionSdkDirRaw.isEmpty()) {
        qCWarning(lcWinRtRunner).nospace()
                << QStringLiteral("The environment variable ExtensionSdkDir is not set.");
        return false;
    }
    const QString extensionSdkDir = QString::fromLocal8Bit(extensionSdkDirRaw);
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
        if (installPackage(dit.filePath()))
            toInstall.remove(name);
    }

    return true;
}

bool AppxEngine::installPackage(const QString &filePath)
{
    Q_D(const AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__ << filePath;

    const QString nativeFilePath = QDir::toNativeSeparators(QFileInfo(filePath).absoluteFilePath());
    const bool addInsteadOfRegister = nativeFilePath.endsWith(QStringLiteral(".appx"),
                                                              Qt::CaseInsensitive);
    HRESULT hr;
    ComPtr<IUriRuntimeClass> uri;
    hr = d->uriFactory->CreateUri(hStringFromQString(nativeFilePath), &uri);
    CHECK_RESULT("Unable to create an URI for the package.", return false);

    ComPtr<IAsyncOperationWithProgress<DeploymentResult *, DeploymentProgress>> deploymentOperation;
    if (addInsteadOfRegister) {
        hr = d->packageManager->AddPackageAsync(uri.Get(), NULL, DeploymentOptions_None,
                                                &deploymentOperation);
        CHECK_RESULT("Unable to add package.", return false);
    } else {
        hr = d->packageManager->RegisterPackageAsync(uri.Get(), 0,
                                                     DeploymentOptions_DevelopmentMode,
                                                     &deploymentOperation);
        CHECK_RESULT("Unable to start package registration.", return false);
    }

    ComPtr<IDeploymentResult> results;
    while ((hr = deploymentOperation->GetResults(&results)) == E_ILLEGAL_METHOD_CALL)
        Sleep(1);

    HRESULT errorCode;
    hr = results->get_ExtendedErrorCode(&errorCode);
    CHECK_RESULT("Unable to retrieve package registration results.", return false);

    if (FAILED(errorCode)) {
        HString errorText;
        if (SUCCEEDED(results->get_ErrorText(errorText.GetAddressOf()))) {
            qCWarning(lcWinRtRunner) << "Unable to register package:"
                                     << QString::fromWCharArray(errorText.GetRawBuffer(NULL));
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

    return installDependencies() && installPackage(d->manifest);
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

    const QString launchArguments =
            (d->runner->arguments() << QStringLiteral("-qdevel")).join(QLatin1Char(' '));
    DWORD pid;
    const QString activationId = d->packageFamilyName + QStringLiteral("!App");
    HRESULT hr = d->appLauncher->ActivateApplication(wchar(activationId),
                                                     wchar(launchArguments), AO_NONE, &pid);
    CHECK_RESULT("Failed to activate application.", return false);
    d->pid = qint64(pid);
    CloseHandle(d->processHandle);
    d->processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, true, pid);

    return true;
}

bool AppxEngine::enableDebugging(const QString &debuggerExecutable, const QString &debuggerArguments)
{
    Q_D(AppxEngine);

    const QString &debuggerCommand = debuggerExecutable + QLatin1Char(' ') + debuggerArguments;
    HRESULT hr = d->packageDebug->EnableDebugging(wchar(d->packageFullName),
                                                  wchar(debuggerCommand),
                                                  NULL);
    CHECK_RESULT("Failed to enable debugging for application.", return false);
    return true;
}

bool AppxEngine::disableDebugging()
{
    Q_D(AppxEngine);

    HRESULT hr = d->packageDebug->DisableDebugging(wchar(d->packageFullName));
    CHECK_RESULT("Failed to disable debugging for application.", return false);

    return true;
}

bool AppxEngine::suspend()
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    HRESULT hr = d->packageDebug->Suspend(wchar(d->packageFullName));
    CHECK_RESULT("Failed to suspend application.", return false);

    return true;
}

bool AppxEngine::waitForFinished(int secs)
{
    Q_D(AppxEngine);
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    debugMonitor->start(d->packageFamilyName);

    g_handleCtrl = true;
    int time = 0;
    forever {
        PACKAGE_EXECUTION_STATE state;
        HRESULT hr = d->packageDebug->GetPackageExecutionState(wchar(d->packageFullName), &state);
        CHECK_RESULT("Failed to get package execution state.", return false);
        qCDebug(lcWinRtRunner) << "Current execution state:" << state;
        if (state == PES_TERMINATED || state == PES_UNKNOWN)
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
        CHECK_RESULT("Failed to terminate package process.", return false);

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

void AppxEnginePrivate::retrieveInstalledPackages()
{
    qCDebug(lcWinRtRunner) << __FUNCTION__;

    ComPtr<ABI::Windows::Foundation::Collections::IIterable<Package*>> packages;
    HRESULT hr = packageManager->FindPackagesByUserSecurityId(NULL, &packages);
    CHECK_RESULT("Failed to find packages.", return);

    ComPtr<ABI::Windows::Foundation::Collections::IIterator<Package*>> pkgit;
    hr = packages->First(&pkgit);
    CHECK_RESULT("Failed to get package iterator.", return);

    boolean hasCurrent;
    hr = pkgit->get_HasCurrent(&hasCurrent);
    while (SUCCEEDED(hr) && hasCurrent) {
        ComPtr<IPackage> pkg;
        hr = pkgit->get_Current(&pkg);
        CHECK_RESULT("Failed to get current package.", return);

        ComPtr<IPackageId> pkgId;
        hr = pkg->get_Id(&pkgId);
        CHECK_RESULT("Failed to get package id.", return);

        HString name;
        hr = pkgId->get_Name(name.GetAddressOf());
        CHECK_RESULT("Failed retrieve package name.", return);

        ProcessorArchitecture architecture;
        if (packageArchitecture == ProcessorArchitecture_Neutral) {
            architecture = packageArchitecture;
        } else {
            hr = pkgId->get_Architecture(&architecture);
            CHECK_RESULT("Failed to retrieve package architecture.", return);
        }

        const QString pkgName = QStringFromHString(name.Get());
        qCDebug(lcWinRtRunner) << "found installed package" << pkgName;

        if (architecture == packageArchitecture)
            installedPackages.insert(pkgName);

        hr = pkgit->MoveNext(&hasCurrent);
    }
}
