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
#include <dbt.h>
#include <setupapi.h>

QT_USE_NAMESPACE

Q_LOGGING_CATEGORY(lcD3DService, "qt.d3dservice")

#define _WIDEN(x) L ## x
#define WIDEN(x) _WIDEN(x)
#define LQT_VERSION_STR WIDEN(QT_VERSION_STR)

// The GUID used by the Windows Phone IP over USB service
static const GUID GUID_DEVICE_WINPHONE8_USB = { 0x26fedc4eL, 0x6ac3, 0x4241, 0x9e, 0x4d, 0xe3, 0xd4, 0xb2, 0xc5, 0xc5, 0x34 };

// Handlers
typedef int (*HandleDeviceFunction)(int, const QString &, const QString &, HANDLE);
extern int handleAppxDevice(int deviceIndex, const QString &app, const QString &cacheDir, HANDLE runLock);
extern int handleXapDevice(int deviceIndex, const QString &app, const QString &cacheDir, HANDLE runLock);
typedef int (*AppListFunction)(int, QSet<QString> &);
extern int xapAppNames(int deviceIndex, QSet<QString> &apps);
extern int appxAppNames(int deviceIndex, QSet<QString> &app);

extern QStringList xapDeviceNames();

// Callbacks
static BOOL __stdcall control(DWORD type);
static LRESULT __stdcall control(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
static DWORD __stdcall deviceWorker(LPVOID param);
static DWORD __stdcall appWorker(LPVOID param);

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
        ushort facility;
        ushort code;
    };
    ulong val;
};

enum ControlEvent
{
    NoCommand = 0, Stop = 1, NewWorker = 2, PhoneConnected = 3
};

struct D3DServicePrivate
{
    D3DServicePrivate()
        : controlEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
        , controlWindow(0)
        , deviceHandle(0)
    {
    }
    ~D3DServicePrivate()
    {
        if (deviceHandle)
            UnregisterDeviceNotification(deviceHandle);
        if (controlWindow)
            CloseHandle(controlWindow);
        if (controlEvent)
            CloseHandle(controlEvent);
    }

    HANDLE controlEvent;
    HWND controlWindow;
    HDEVNOTIFY deviceHandle;
    QList<ControlEvent> eventQueue;
    QList<QStringPair> workerQueue;
};
Q_GLOBAL_STATIC(D3DServicePrivate, d)

struct WorkerParam
{
    enum { NoError = 0, GeneralError = 1, BadDeviceIndex = 2, NoCacheDir = 3 };
    WorkerParam(const QString &deviceName, const QString &app, HANDLE runLock)
        : deviceName(deviceName), app(app), runLock(runLock) { }
    QString deviceName;
    QString app;
    HANDLE runLock;
};

class Worker
{
public:
    Worker(const QStringPair &config, LPTHREAD_START_ROUTINE worker)
        : m_runLock(CreateEvent(NULL, FALSE, FALSE, NULL))
        , m_param(config.first, config.second, m_runLock)
        , m_thread(CreateThread(NULL, 0, worker, &m_param, 0, NULL))
    {
    }
    ~Worker()
    {
        SetEvent(m_runLock);
        WaitForSingleObject(m_thread, INFINITE);
        CloseHandle(m_runLock);
        CloseHandle(m_thread);
    }
    HANDLE thread() const
    {
        return m_thread;
    }
private:
    HANDLE m_runLock;
    WorkerParam m_param;
    HANDLE m_thread;
};

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
    return QDir::toNativeSeparators(baseDir.absolutePath());
}

bool D3DService::start()
{
    HANDLE runLock = CreateMutex(NULL, TRUE, L"Local\\qtd3dservice");
    if (!runLock || GetLastError() == ERROR_ALREADY_EXISTS) {
        qCWarning(lcD3DService) << "The service is already running.";
        return false;
    }

    SetConsoleCtrlHandler(&control, TRUE);

    // Create an invisible window for getting broadcast events
    WNDCLASS controlWindowClass = { 0, &control, 0, 0, NULL, NULL,
                                    NULL, NULL, NULL, L"controlWindow" };
    if (!RegisterClass(&controlWindowClass)) {
        qCCritical(lcD3DService) << "Unable to register control window class:"
                                 << qt_error_string(GetLastError());
        return false;
    }
    d->controlWindow = CreateWindowEx(0, L"controlWindow", NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);

    // Register for USB notifications
    DEV_BROADCAST_DEVICEINTERFACE filter = {
        sizeof(DEV_BROADCAST_DEVICEINTERFACE), DBT_DEVTYP_DEVICEINTERFACE,
        0, GUID_DEVICE_WINPHONE8_USB, 0
    };
    d->deviceHandle = RegisterDeviceNotification(d->controlWindow, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    QVector<HANDLE> waitHandles;
    waitHandles.append(d->controlEvent);

    // Dummy handle for phone (gets replaced by a worker when needed)
    HANDLE dummyHandle;
    DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(),
                    GetCurrentProcess(), &dummyHandle, SYNCHRONIZE, FALSE, 0);
    waitHandles.append(dummyHandle);

    // Event handles for emulators (XDE)
    WCHAR username[MAX_PATH];
    ULONG usernameSize = MAX_PATH;
    GetUserName(username, &usernameSize);
    const QStringList emulatorNames = xapDeviceNames().mid(1);
    foreach (const QString &name, emulatorNames) {
        const QString eventName = QStringLiteral("Local\\XdeOnServerInitialize")
                + name + QLatin1Char('.') + QString::fromWCharArray(username, usernameSize).toLower();
        HANDLE event = CreateEvent(NULL, TRUE, FALSE, reinterpret_cast<LPCWSTR>(eventName.utf16()));
        if (event)
            waitHandles.append(event);
        else
            qCWarning(lcD3DService) << "Unable to create event:" << qt_error_string(GetLastError());
    }

    // App monitoring threads
    QHash<QStringPair, Worker *> workers;
    QHash<HANDLE, QStringPair> workerThreads;

    // Device monitoring threads - one per device
    QVector<Worker *> deviceWorkers(emulatorNames.size() + 1, NULL);

    // If a Windows Phone is already connected, queue a device worker
    HDEVINFO info = SetupDiGetClassDevs(&GUID_DEVICE_WINPHONE8_USB, NULL, NULL,
                                        DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
    if (info != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA infoData = { sizeof(SP_DEVINFO_DATA) };
        if (SetupDiEnumDeviceInfo(info, 0, &infoData)) {
            d->eventQueue.append(PhoneConnected);
            SetEvent(d->controlEvent);
        }
        SetupDiDestroyDeviceInfoList(info);
    }

    // Create a monitoring thread for local Appx packages
    Worker appxWorker(qMakePair(QStringLiteral("local"), QString()), &deviceWorker);
    Q_UNUSED(appxWorker);

    // Master loop
    // This loop handles incoming events from the service controller and
    // worker threads. It also creates new worker threads as needed.
    const uint phoneEvent = WAIT_OBJECT_0 + 1;
    const uint minEmulatorEvent = phoneEvent + 1;
    const uint maxEmulatorEvent = minEmulatorEvent + emulatorNames.size() - 1;
    const uint minWorker = maxEmulatorEvent + 1;
    uint maxWorker = minWorker + workers.size() - 1;
    forever {
        DWORD event = MsgWaitForMultipleObjects(waitHandles.size(), waitHandles.data(), FALSE, INFINITE, QS_ALLINPUT);
        if (event >= WAIT_OBJECT_0 && event < WAIT_OBJECT_0 + waitHandles.size()) {
            // A control event occurred
            if (event == WAIT_OBJECT_0) {
                bool shutdown = false;
                while (!d->eventQueue.isEmpty()) {
                    ControlEvent controlEvent = d->eventQueue.takeFirst();

                    // Break out and shutdown
                    if (controlEvent == Stop) {
                        shutdown = true;
                        break;
                    }

                    // A new worker is in the queue
                    if (controlEvent == NewWorker) {
                        while (!d->workerQueue.isEmpty()) {
                            const QStringPair config = d->workerQueue.takeFirst();
                            if (workers.contains(config)) { // The config is already running
                                qCDebug(lcD3DService) << "Discarded worker configuration:"
                                                      << config.first << config.second;
                                continue;
                            }

                            Worker *worker = new Worker(config, &appWorker);
                            workers.insert(config, worker);
                            workerThreads.insert(worker->thread(), config);
                            waitHandles.append(worker->thread());
                            maxWorker = minWorker + workers.size() - 1;
                        }
                        continue;
                    }

                    // A Windows Phone device was connected
                    if (controlEvent == PhoneConnected) {
                        qCDebug(lcD3DService) << "A Windows Phone has connected.";
                        // The worker is already active
                        if (deviceWorkers.first())
                            continue;

                        // Start the phone monitoring thread
                        Worker *worker = new Worker(qMakePair(QString::number(0), QString()), &deviceWorker);
                        deviceWorkers[0] = worker;
                        CloseHandle(waitHandles[phoneEvent - WAIT_OBJECT_0]);
                        waitHandles[phoneEvent - WAIT_OBJECT_0] = worker->thread();
                        continue;
                    }
                }

                if (shutdown)
                    break;

                continue;
            }

            // Device events
            if (event >= phoneEvent && event <= maxEmulatorEvent) {
                const int deviceIndex = event - phoneEvent;
                // Determine if the handle in the slot is an event or thread
                if (GetThreadId(waitHandles[event - WAIT_OBJECT_0])) {
                    qCDebug(lcD3DService) << "Device worker exited:" << deviceIndex;
                    // The thread has exited, close the handle and replace the event
                    delete deviceWorkers.at(deviceIndex);
                    deviceWorkers[deviceIndex] = 0;

                    // The phone case is handled elsewhere; set a dummy handle
                    if (event == phoneEvent) {
                        HANDLE dummyHandle;
                        DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(),
                                        GetCurrentProcess(), &dummyHandle, SYNCHRONIZE, FALSE, 0);
                        waitHandles[event - WAIT_OBJECT_0] = dummyHandle;
                        continue;
                    }

                    // Re-create the event handle
                    const QString eventName = QStringLiteral("Local\\XdeOnServerInitialize")
                            + emulatorNames.at(deviceIndex - 1) + QLatin1Char('.')
                            + QString::fromWCharArray(username, usernameSize).toLower();
                    HANDLE emulatorEvent = CreateEvent(
                                NULL, TRUE, FALSE, reinterpret_cast<LPCWSTR>(eventName.utf16()));
                    if (emulatorEvent) {
                        waitHandles[event - WAIT_OBJECT_0] = emulatorEvent;
                    } else {
                        // If the above fails, replace it with a dummy to keep things going
                        qCCritical(lcD3DService).nospace() << "Unable to create event for emulator "
                                                           << emulatorNames.at(deviceIndex - 1)
                                                           << ": " << qt_error_string(GetLastError());
                        HANDLE dummyHandle;
                        DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(),
                                        GetCurrentProcess(), &dummyHandle, SYNCHRONIZE, FALSE, 0);
                        waitHandles[event - WAIT_OBJECT_0] = dummyHandle;
                    }
                } else {
                    qCDebug(lcD3DService) << "An emulator was activated:" << deviceIndex;
                    // The event was set; close the handle and replace with a thread
                    CloseHandle(waitHandles[event - WAIT_OBJECT_0]);

                    // This shouldn't happen
                    if (event == phoneEvent)
                        continue;

                    const QStringPair config = qMakePair(QString::number(deviceIndex), QString());
                    Worker *worker = new Worker(config, &deviceWorker);
                    deviceWorkers[deviceIndex] = worker;
                    waitHandles[event - WAIT_OBJECT_0] = worker->thread();
                }
                continue;
            }

            // A worker exited
            if (event >= minWorker && event <= maxWorker) {
                // Delete the worker and clear the handle (TODO: remove it?)
                HANDLE thread = waitHandles.takeAt(event - WAIT_OBJECT_0);
                QStringPair config = workerThreads.take(thread);
                Worker *worker = workers.take(config);
                delete worker;
                continue;
            }
        }

        // TODO: check return val for this
        MSG msg;
        if (PeekMessage(&msg, d->controlWindow, 0, 0, PM_REMOVE))
            DispatchMessage(&msg);
    }

    qDeleteAll(workers);

    // Close the phone and emulator handles
    for (int i = 0; i <= emulatorNames.size(); ++i) {
        if (GetThreadId(waitHandles[i + 1]))
            delete deviceWorkers.at(i);
        else
            CloseHandle(waitHandles[i + 1]);
    }

    CloseHandle(runLock);

    return true;
}

DWORD __stdcall deviceWorker(LPVOID param)
{
    WorkerParam *args = reinterpret_cast<WorkerParam *>(param);

    // The list of applications on this device will be polled until
    // an error code is returned (e.g. the device is disconnected) or
    // the thread is told to exit.
    QSet<QString> appNames;
    AppListFunction appList;

    int deviceIndex = 0;
    HKEY waitKey = 0;
    if (args->deviceName.isEmpty() || args->deviceName == QStringLiteral("local")) {
        appList = appxAppNames;
        LONG result = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\PackageRepository\\Packages",
                    0, KEY_NOTIFY, &waitKey);
        if (result != ERROR_SUCCESS) {
            qCWarning(lcD3DService) << "Unable to open registry key for Appx discovery:"
                                    << qt_error_string(result);
            waitKey = 0;
        }
    } else {
        // CoreCon (Windows Phone)
        bool ok;
        deviceIndex = args->deviceName.toInt(&ok);
        if (!ok)
            return WorkerParam::BadDeviceIndex;

        appList = xapAppNames;
    }

    forever {
        if (WaitForSingleObject(args->runLock, 0) == WAIT_OBJECT_0)
            return 0;

        QSet<QString> latestAppNames;
        int exitCode = appList(deviceIndex, latestAppNames);
        if (exitCode != WorkerParam::NoError)
            return exitCode;

        QSet<QString> newAppNames = latestAppNames - appNames;
        if (!newAppNames.isEmpty()) {
            // Create a new app watcher for each new app
            foreach (const QString &app, newAppNames) {
                qCWarning(lcD3DService).nospace() << "Found app " << app << " on device "
                                                  << args->deviceName << '.';
                d->workerQueue.append(qMakePair(args->deviceName, app));
            }

            d->eventQueue.append(NewWorker);
            SetEvent(d->controlEvent);
        }

        appNames = latestAppNames;

        // If possible, wait for the registry event (otherwise, go straight to sleep)
        if (waitKey) {
            HANDLE waitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            LONG result = RegNotifyChangeKeyValue(waitKey, TRUE, REG_NOTIFY_CHANGE_NAME, waitEvent, TRUE);
            if (result != ERROR_SUCCESS) {
                qCWarning(lcD3DService) << "Unable to create registry notifier:"
                                        << qt_error_string(result);
                RegCloseKey(waitKey); // Revert to polling
                waitKey = 0;
            }

            HANDLE waitHandles[] = { args->runLock, waitEvent };
            result = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
            CloseHandle(waitEvent);
            if (result == WAIT_OBJECT_0) // runLock, exit
                return 0;
            if (result == WAIT_OBJECT_0 + 1) { // registry changed, reset app list
                appNames.clear();
            } else {
                qCWarning(lcD3DService) << "Unexpected wait result:" << result
                                        << qt_error_string(GetLastError());
                RegCloseKey(waitKey); // Revert to polling
                waitKey = 0;
            }
        }
        Sleep(1000);
    }

    if (waitKey)
        RegCloseKey(waitKey);
    return 0;
}

DWORD __stdcall appWorker(LPVOID param)
{
    WorkerParam *args = reinterpret_cast<WorkerParam *>(param);

    HandleDeviceFunction handleDevice;
    QString cachePath;
    int deviceIndex = 0;
    // Simple case, local device
    if (args->deviceName.isEmpty() || args->deviceName == QStringLiteral("local")) {
        cachePath = prepareCache(QStringLiteral("local"), args->app);
        if (cachePath.isEmpty()) {
            qCCritical(lcD3DService) << "Unable to create local shader cache.";
            return WorkerParam::NoCacheDir;
        }
        handleDevice = &handleAppxDevice;
    } else {
        // CoreCon (Windows Phone) case
        bool ok;
        deviceIndex = args->deviceName.toInt(&ok);
        if (!ok)
            return WorkerParam::BadDeviceIndex;
        cachePath = prepareCache(args->deviceName, args->app);
        if (cachePath.isEmpty()) {
            qCCritical(lcD3DService) << "Unable to create local shader cache.";
            return WorkerParam::NoCacheDir;
        }
        handleDevice = &handleXapDevice;
    }

    return handleDevice(deviceIndex, args->app, cachePath, args->runLock);
}

// Console message controller
LRESULT __stdcall control(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DEVICECHANGE && wParam == DBT_DEVICEARRIVAL) {
        DEV_BROADCAST_DEVICEINTERFACE *header =
                reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE *>(lParam);
        if (header->dbcc_classguid == GUID_DEVICE_WINPHONE8_USB) {
            d->eventQueue.append(PhoneConnected);
            SetEvent(d->controlEvent);
        }
    }
    return DefWindowProc(window, msg, wParam, lParam);
}

// Console CTRL controller
BOOL __stdcall control(DWORD type)
{
    switch (type) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT: {
        d->eventQueue.append(Stop);
        SetEvent(d->controlEvent);
        return true;
    }
    // fall through
    case CTRL_BREAK_EVENT:
    default:
        break;
    }
    return false;
}
