// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BASEDEVICE_H
#define BASEDEVICE_H

class BaseDevice
{
public:
    BaseDevice();
    virtual ~BaseDevice();

    virtual void internalVirtualFunction();

    virtual void regularVirtualFunction();

    virtual void pureVirtualFunction() = 0;

protected:
    virtual void protectedInternalFunction();

private:
    virtual void privateVirtualFunction();
};

#endif // BASEDEVICE_H

