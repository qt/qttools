// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "networkdevice.h"

/*!
    \class NetworkDevice
    \inmodule TestModule
    \brief Derived class that reimplements base class methods.

    This class demonstrates the reimplemented member visibility issue.
    Some reimplemented functions should appear in "Reimplemented Functions"
    sections even when the base class method is marked as internal.
*/

/*!
    Constructs a NetworkDevice.
*/
NetworkDevice::NetworkDevice()
{
}

/*!
    Destroys the NetworkDevice.
*/
NetworkDevice::~NetworkDevice()
{
}

/*!
    \internal
*/
void NetworkDevice::internalVirtualFunction()
{
    // NetworkDevice implementation
}

/*!
    \reimp
*/
void NetworkDevice::regularVirtualFunction()
{
    // NetworkDevice implementation
}

/*!
    \internal
*/
void NetworkDevice::pureVirtualFunction()
{
    // NetworkDevice implementation
}

/*!
    \internal
*/
void NetworkDevice::protectedInternalFunction()
{
    // NetworkDevice implementation
}

void NetworkDevice::privateVirtualFunction()
{
    // NetworkDevice implementation
}

