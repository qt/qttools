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

#include <QtCore/QCommandLineParser>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QBitArray>

#include <iostream>
#include <thread>

QT_USE_NAMESPACE

Q_LOGGING_CATEGORY(lcD3DService, "qt.d3dservice")

#define _WIDEN(x) L ## x
#define WIDEN(x) _WIDEN(x)
#define LQT_VERSION_STR WIDEN(QT_VERSION_STR)

// Handlers
extern void handleAppxDevice(int deviceIndex, const QString &app, const QString &cacheDir);
extern void handleXapDevice(int deviceIndex, const QString &app, const QString &cacheDir);

// Callbacks
static void __stdcall run(DWORD argc, LPWSTR argv[]);
static void __stdcall control(DWORD code);

union ErrorId
{
    enum FacilityFlag {
        Success = 0x0,
        Informational = 0x40000000,
        Warning = 0x80000000,
        Error = Informational|Warning,
        Customer = 0x20000000
    };
    struct {
        ushort facility : 2;
        ushort code : 2;
    };
    ulong val;
};

// This class instance is shared between all runners
struct D3DServiceShared
{
    D3DServiceShared()
        : name(L"qtd3dservice")
    {
        GetModuleFileName(NULL, path, MAX_PATH);
        registrations = D3DService::registrations();
    }

    const wchar_t *name;
    wchar_t path[MAX_PATH];
    QList<QPair<QString, QString>> registrations;
};
Q_GLOBAL_STATIC(D3DServiceShared, shared)

// One instance of this class is created for each runner
struct D3DServicePrivate
{
    D3DServicePrivate() : stopEvent(NULL), isService(false), checkPoint(1)
    {
        status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
        status.dwServiceSpecificExitCode = 0;
    }

    HANDLE stopEvent;
    bool isService;
    SERVICE_STATUS status;
    SERVICE_STATUS_HANDLE statusHandle;
    DWORD checkPoint;
};
// Unrolled Q_GLOBAL_STATIC to add __declspec(thread) to the declaration
namespace {
    namespace Q_QGS_d {
        typedef D3DServicePrivate Type;
        QBasicAtomicInt guard = Q_BASIC_ATOMIC_INITIALIZER(QtGlobalStatic::Uninitialized);
        Q_GLOBAL_STATIC_INTERNAL(())
    }
}
static __declspec(thread) QGlobalStatic<D3DServicePrivate, Q_QGS_d::innerFunction, Q_QGS_d::guard> d;

void d3dserviceMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &text)
{
    LPCWSTR strings[2] = { shared->name, reinterpret_cast<const wchar_t *>(text.utf16()) };
    HANDLE eventSource = RegisterEventSource(NULL, shared->name);
    if (eventSource) {
        if (type > QtDebugMsg) {
            ErrorId id = { ushort(FACILITY_NULL | ErrorId::Customer), type };
            switch (type) {
            default:
            case 1:
                id.facility |= ErrorId::Informational;
                break;
            case 2:
                id.facility |= ErrorId::Warning;
                break;
            case 3:
                id.facility |= ErrorId::Error;
                break;
            }
            ReportEvent(eventSource, type, 0, id.val, NULL, 2, 0, strings, NULL);
            DeregisterEventSource(eventSource);
        }
    }
}

static QString prepareCache(const QString &device, const QString &app)
{
    // Make sure we have a writable cache
    QDir baseDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (!baseDir.mkpath(QStringLiteral("qtd3dservice")))
        return QString();
    baseDir.cd(QStringLiteral("qtd3dservice"));
    if (!baseDir.mkpath(device))
        return QString();
    baseDir.cd(device);
    if (!baseDir.mkpath(app))
        return QString();
    baseDir.cd(app);
    if (!baseDir.mkpath(QStringLiteral("source")))
        return QString();
    if (!baseDir.mkpath(QStringLiteral("binary")))
        return QString();
    QFile controlFile(baseDir.absoluteFilePath(QStringLiteral("control")));
    if (!controlFile.open(QFile::WriteOnly))
        return QString();
    controlFile.write(QByteArrayLiteral("Qt D3D shader compilation service"));
    return QDir::toNativeSeparators(baseDir.absolutePath());
}

bool D3DService::install()
{
    SC_HANDLE manager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);
    if (!manager) {
        qCWarning(lcD3DService) << qt_error_string(GetLastError());
        qCWarning(lcD3DService) << "When installing, run this program as an administrator.";
        return 0;
    }

    // Create the service
    SC_HANDLE service = CreateService(manager, shared->name, L"Qt D3D Compiler Service " LQT_VERSION_STR,
                                      SERVICE_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS,
                                      SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                                      shared->path, NULL, NULL, NULL, NULL, NULL);
    if (!service) {
        qCWarning(lcD3DService) << qt_error_string(GetLastError());
        switch (GetLastError()) {
        case ERROR_SERVICE_EXISTS:
            qCWarning(lcD3DService) << "Please remove the service before reinstalling.";
            break;
        default:
            qCWarning(lcD3DService) << "Failed to install the service.";
            break;
        }
        CloseServiceHandle(manager);
        return false;
    }

    qCWarning(lcD3DService) << "Service installation successful.";

    // Do some more stuff

    CloseServiceHandle(service);
    CloseServiceHandle(manager);
    return true;
}

bool D3DService::remove()
{
    SC_HANDLE manager = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, DELETE);
    if (!manager) {
        qCWarning(lcD3DService) << qt_error_string(GetLastError());
        qCWarning(lcD3DService) << "When removing, run this program as an administrator.";
        return 0;
    }

    // Get a handle to the SCM database.
    SC_HANDLE service = OpenService(manager, shared->name, DELETE);
    if (!service) {
        // System message
        qCWarning(lcD3DService) << qt_error_string(GetLastError());
        // Friendly message
        switch (GetLastError()) {
        case ERROR_ACCESS_DENIED:
            qCWarning(lcD3DService) << "When removing, run this program as an administrator.";
            break;
        default:
            qCWarning(lcD3DService) << "Failed to remove the service.";
            break;
        }
        CloseServiceHandle(manager);
        return false;
    }

    if (!DeleteService(service)) {
        qCWarning(lcD3DService) << qt_error_string(GetLastError());
        qCWarning(lcD3DService) << "Unable to remove the service.";
        CloseServiceHandle(service);
        CloseServiceHandle(manager);
        return false;
    }

    qCWarning(lcD3DService) << "Service removal successful.";
    CloseServiceHandle(service);
    CloseServiceHandle(manager);
    return true;
}

bool D3DService::startService()
{
    int registrationCount = shared->registrations.count();
    if (!registrationCount) {
        // If there are no registrations, we'll run once and exit
        SERVICE_TABLE_ENTRY dispatchTableStub[] = {
            { const_cast<wchar_t *>(shared->name), &run },
            { NULL, NULL }
        };
        return StartServiceCtrlDispatcher(dispatchTableStub);
    }
    QVector<SERVICE_TABLE_ENTRY> dispatchTable(registrationCount + 1);
    for (int i = 0; i < registrationCount; ++i)
        dispatchTable[i] = { const_cast<wchar_t *>(shared->name), &run };
    dispatchTable[registrationCount] = { NULL, NULL };
    return StartServiceCtrlDispatcher(dispatchTable.data());
}

bool D3DService::startDirectly()
{
    if (shared->registrations.isEmpty())
        return true;

    // Create one worker per registration
    int threadCount = shared->registrations.count();
    QVector<std::thread *> threads(threadCount);
    for (int i = 0; i < threadCount; ++i) {
        LPWSTR *args = 0;
        threads[i] = new std::thread(&run, 0, args);
    }
    for (int i = 0; i < threadCount; ++i) {
        threads[i]->join();
        delete threads[i];
    }

    return true;
}

void D3DService::reportStatus(DWORD state, DWORD exitCode, DWORD waitHint)
{
    if (!d->isService)
        return;

    d->status.dwCurrentState = state;
    d->status.dwWin32ExitCode = exitCode;
    d->status.dwWaitHint = waitHint;
    d->status.dwControlsAccepted = state == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP;
    d->status.dwCheckPoint = (state == SERVICE_RUNNING || state == SERVICE_STOPPED) ? 0 : d->checkPoint++;
    SetServiceStatus(d->statusHandle, &d->status);
}

void __stdcall run(DWORD argc, LPWSTR argv[])
{
    if (argc && lstrcmp(argv[0], shared->name) == 0) {
        d->isService = true;
#if defined(_DEBUG)
        // Debugging aid. Gives 15 seconds after startup to attach a debuggger.
        // The SCM will give the service 30 seconds to start.
        int count = 0;
        while (!IsDebuggerPresent() && count++ < 15)
            Sleep(1000);
#endif
        qInstallMessageHandler(&d3dserviceMessageHandler);
    }

    if (d->isService) {
        d->stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        d->statusHandle = RegisterServiceCtrlHandler(shared->name, &control);
        D3DService::reportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
    }

    if (shared->registrations.isEmpty()) {
        qCWarning(lcD3DService) << "No configuration available - have you registered applications to be monitored?";
        D3DService::reportStatus(SERVICE_RUNNING, NO_ERROR, 0);
        D3DService::reportStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        D3DService::reportStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    QPair<QString, QString> config = shared->registrations.takeFirst();
    QString deviceName = config.first;
    QString app = config.second;

    // Simple case, local device
    if (deviceName.isEmpty() || deviceName == QStringLiteral("local")) {
        const QString cachePath = prepareCache(QStringLiteral("local"), app);
        if (cachePath.isEmpty()) {
            qCCritical(lcD3DService) << "Unable to create local shader cache.";
            return;
        }
        handleAppxDevice(0, app, cachePath);
        D3DService::reportStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    // CoreCon (Windows Phone) case
    bool ok;
    int deviceIndex = deviceName.toInt(&ok);
    if (!ok) {
        D3DService::reportStatus(SERVICE_RUNNING, NO_ERROR, 0);
        return;
    }

    const QString cachePath = prepareCache(deviceName, app);
    if (cachePath.isEmpty()) {
        qCCritical(lcD3DService) << "Unable to create local shader cache.";
        return;
    }
    handleXapDevice(deviceIndex, app, cachePath);
    D3DService::reportStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

void __stdcall control(DWORD code)
{
    switch (code) {
      case SERVICE_CONTROL_STOP:
         //ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
         // Signal the service to stop.
         SetEvent(d->stopEvent);
         //ReportSvcStatus(gShared->status.dwCurrentState, NO_ERROR, 0);
         return;
      case SERVICE_CONTROL_INTERROGATE:
         break;
      default:
         break;
   }
}
