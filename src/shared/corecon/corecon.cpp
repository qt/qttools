/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ccapi.h"
#include "corecon.h"

#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <comdef.h>
#include <wrl.h>
using namespace Microsoft::WRL;

QT_USE_NAMESPACE

Q_LOGGING_CATEGORY(lcCoreCon, "qt.corecon")

#define wchar(str) reinterpret_cast<LPCWSTR>(str.utf16())

template <typename T>
static inline HRESULT collectionFor(const ComPtr<T> &container, ComPtr<ICcCollection> &collection)
{
    ComPtr<ICcObjectContainer> objectContainer;
    HRESULT hr = container.As(&objectContainer);
    if (FAILED(hr))
        return hr;
    hr = objectContainer->EnumerateObjects(&collection);
    return hr;
}

class CoreConDevicePrivate
{
public:
    QString name;
    QString id;
    bool isEmulator;
    ComPtr<ICcDevice> handle;
};

CoreConDevice::CoreConDevice()
    : d(new CoreConDevicePrivate)
{
}

CoreConDevice::~CoreConDevice()
{
}

QString CoreConDevice::name() const
{
    return d->name;
}

QString CoreConDevice::id() const
{
    return d->id;
}

bool CoreConDevice::isEmulator() const
{
    return d->isEmulator;
}

ICcDevice *CoreConDevice::handle() const
{
    return d->handle.Get();
}

class ComInitializer
{
protected:
    ComInitializer()
    {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
            qCDebug(lcCoreCon) << "Failed to initialize COM.";
    }
    ~ComInitializer()
    {
        if (SUCCEEDED(hr))
            CoUninitialize();
    }
    HRESULT hr;
};

class CoreConServerPrivate : private ComInitializer
{
public:
    CoreConServerPrivate() : langModule(0)
    {
    }
    ~CoreConServerPrivate()
    {
        qDeleteAll(devices);
        devices.clear();
    }
    ComPtr<ICcServer> handle;
    QList<CoreConDevice *> devices;
    HMODULE langModule;
    QMutex mutex;
};

CoreConServer::CoreConServer() : d(new CoreConServerPrivate)
{
    QMutexLocker(&d->mutex);
    HRESULT hr = CoCreateInstance(CLSID_ConMan, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d->handle));
    if (FAILED(hr))
        qCWarning(lcCoreCon) << "Failed to initialize connection server." << formatError(hr);

    // The language module is available as long as the above succeeded
    d->langModule = GetModuleHandle(L"conmanui");
}

CoreConServer::~CoreConServer()
{
}

ICcServer *CoreConServer::handle() const
{
    return d->handle.Get();
}

QList<CoreConDevice *> CoreConServer::devices() const
{
    qCDebug(lcCoreCon) << __FUNCTION__;

    while (!d)
        Sleep(1);

    QMutexLocker(&d->mutex);

    if (!d->devices.isEmpty() || !d->handle)
        return d->devices;

    ComPtr<ICcDatastore> dataStore;
    HRESULT hr = d->handle->GetDatastore(GetUserDefaultLCID(), &dataStore);
    if (FAILED(hr)) {
        qCDebug(lcCoreCon, "Failed to obtain the data store. HRESULT: 0x%x", hr);
        return d->devices;
    }

    ComPtr<ICcPlatformContainer> platformContainer;
    hr = dataStore->get_PlatformContainer(&platformContainer);
    if (FAILED(hr)) {
        qCDebug(lcCoreCon, "Failed to obtain the platform container. HRESULT: 0x%x", hr);
        return d->devices;
    }

    ComPtr<ICcCollection> platformCollection;
    hr = collectionFor(platformContainer, platformCollection);
    if (FAILED(hr)) {
        qCDebug(lcCoreCon, "Failed to obtain the platform collection. HRESULT: 0x%x", hr);
        return d->devices;
    }

    long platformCount;
    hr = platformCollection->get_Count(&platformCount);
    if (FAILED(hr)) {
        qCDebug(lcCoreCon, "Failed to obtain the platform object count. HRESULT: 0x%x", hr);
        return d->devices;
    }
    for (long platformIndex = 0; platformIndex < platformCount; ++platformIndex) {
        ComPtr<ICcObject> platformObject;
        hr = platformCollection->get_Item(platformIndex, &platformObject);
        if (FAILED(hr)) {
            qCDebug(lcCoreCon, "\1: %d", platformIndex);
            continue;
        }

        ComPtr<ICcPlatform> platform;
        hr = platformObject.As(&platform);
        if (FAILED(hr)) {
            qCDebug(lcCoreCon, "\1: %d", platformIndex);
            continue;
        }

        ComPtr<ICcDeviceContainer> deviceContainer;
        hr = platform->get_DeviceContainer(&deviceContainer);
        if (FAILED(hr)) {
            qCDebug(lcCoreCon, "Failed to obtain the device container.. 0x%x", hr);
            continue;
        }

        ComPtr<ICcCollection> deviceCollection;
        hr = collectionFor(deviceContainer, deviceCollection);
        if (FAILED(hr)) {
            qCDebug(lcCoreCon, "Failed to obtain the device object collection.. 0x%x", hr);
            continue;
        }

        long deviceCount;
        hr = deviceCollection->get_Count(&deviceCount);
        if (FAILED(hr)) {
            qCDebug(lcCoreCon, "Failed to obtain the device object count.. 0x%x", hr);
            continue;
        }
        for (long deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
            QScopedPointer<CoreConDevice> device(new CoreConDevice);

            ComPtr<ICcObject> deviceObject;
            hr = deviceCollection->get_Item(deviceIndex, &deviceObject);
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to obtain the device object at index: %d", deviceIndex);
                continue;
            }

            hr = deviceObject.As(&device->d->handle);
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to confirm a device from the object at index: %d", deviceIndex);
                continue;
            }

            _bstr_t deviceId;
            hr = deviceObject->get_ID(deviceId.GetAddress());
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to obtain device id at index: %d", deviceIndex);
                continue;
            }
            device->d->id = QString::fromWCharArray(deviceId);
            _bstr_t deviceName;
            hr = deviceObject->get_Name(deviceName.GetAddress());
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to obtain device name at index: %d", deviceIndex);
                continue;
            }
            device->d->name = QString::fromWCharArray(deviceName);

            ComPtr<ICcPropertyContainer> propertyContainer;
            hr = deviceObject->get_PropertyContainer(&propertyContainer);
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to obtain a property container at index: %d", deviceIndex);
                continue;
            }

            ComPtr<ICcCollection> propertyCollection;
            hr = collectionFor(propertyContainer, propertyCollection);
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to obtain property collection of device at index: %d", deviceIndex);
                continue;
            }

            bool isPseudoDevice = false;
            long propertyCount;
            hr = propertyCollection->get_Count(&propertyCount);
            if (FAILED(hr)) {
                qCDebug(lcCoreCon, "Failed to obtain property count of device at index: %d", deviceIndex);
                continue;
            }

            for (long propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex) {
                ComPtr<ICcObject> propertyObject;
                hr = propertyCollection->get_Item(propertyIndex, &propertyObject);
                if (FAILED(hr)) {
                    qCDebug(lcCoreCon, "Failed to obtain property at index: %d", propertyIndex);
                    continue;
                }

                _bstr_t id;
                hr = propertyObject->get_ID(id.GetAddress());
                if (FAILED(hr)) {
                    qCDebug(lcCoreCon, "Failed to obtain property id at index: %d", propertyIndex);
                    continue;
                }

                ComPtr<ICcProperty> property;
                hr = propertyObject.As(&property);
                if (FAILED(hr)) {
                    qCDebug(lcCoreCon, "Failed to cast the property object at index: %d", propertyIndex);
                    continue;
                }

                if (id == _bstr_t(L"IsPseudoDevice")) {
                    _bstr_t value;
                    hr = property->get_Value(value.GetAddress());
                    if (FAILED(hr)) {
                        qCDebug(lcCoreCon, "Failed to cast the property value at index: %d", propertyIndex);
                        continue;
                    }
                    if (value == _bstr_t(L"true")) {
                        isPseudoDevice = true;
                        break; // No need to look at this device further
                    }
                }

                if (id == _bstr_t(L"Emulator")) {
                    _bstr_t value;
                    hr = property->get_Value(value.GetAddress());
                    if (FAILED(hr)) {
                        qCDebug(lcCoreCon, "Failed to cast the property value at index: %d", propertyIndex);
                        continue;
                    }
                    device->d->isEmulator = value == _bstr_t(L"true");
                }
            }


            if (!isPseudoDevice)
                d->devices.append(device.take());
        }
    }
    return d->devices;
}

QString CoreConServer::formatError(HRESULT hr) const
{
    wchar_t error[1024];
    HMODULE module = 0;
    DWORD origin = HRESULT_FACILITY(hr);
    if (origin == 0x974)
        module = d->langModule;
    if (module)
        LoadString(module, HRESULT_CODE(hr), error, sizeof(error)/sizeof(wchar_t));
    else
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, error, sizeof(error)/sizeof(wchar_t), NULL);
    return QString::fromWCharArray(error).trimmed();
}
