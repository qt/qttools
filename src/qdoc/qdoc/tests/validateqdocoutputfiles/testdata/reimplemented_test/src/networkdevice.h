// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef NETWORKDEVICE_H
#define NETWORKDEVICE_H

#include "basedevice.h"

class NetworkDevice : public BaseDevice
{
public:
    NetworkDevice();
    ~NetworkDevice() override;

    void internalVirtualFunction() override;

    void regularVirtualFunction() override;

    void pureVirtualFunction() override;

protected:
    void protectedInternalFunction() override;

private:
    void privateVirtualFunction() override;
};

#endif // NETWORKDEVICE_H

