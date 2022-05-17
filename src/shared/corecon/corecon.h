// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CORECON_H
#define CORECON_H

#include <QtCore/qt_windows.h>
#include <QtCore/QList>
#include <QtCore/QScopedPointer>
#include <QtCore/QLoggingCategory>

QT_USE_NAMESPACE

class CoreConDevicePrivate;
class CoreConDevice
{
public:
    explicit CoreConDevice(int version);
    ~CoreConDevice();
    QString name() const;
    QString id() const;
    bool isEmulator() const;
    Qt::HANDLE handle() const;
private:
    QScopedPointer<CoreConDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(CoreConDevice)
friend class CoreConServerPrivate;
};

class CoreConServerPrivate;
class CoreConServer
{
public:
    explicit CoreConServer(int version);
    ~CoreConServer();
    bool initialize();
    Qt::HANDLE handle() const;
    QList<CoreConDevice *> devices() const;
    QString formatError(HRESULT hr) const;
private:
    QScopedPointer<CoreConServerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(CoreConServer)
};

Q_DECLARE_LOGGING_CATEGORY(lcCoreCon)

#endif // CORECON_H
