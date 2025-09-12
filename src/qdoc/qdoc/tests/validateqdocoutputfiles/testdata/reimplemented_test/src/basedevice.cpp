// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "basedevice.h"

/*!
    \class BaseDevice
    \inmodule TestModule
    \brief Base class demonstrating reimplemented member visibility.

    This class provides virtual methods that will be reimplemented
    in derived classes, with some marked as internal.
*/

/*!
    Constructs a BaseDevice.
*/
BaseDevice::BaseDevice()
{
}

/*!
    Destroys the BaseDevice.
*/
BaseDevice::~BaseDevice()
{
}

/*!
    \internal
*/
void BaseDevice::internalVirtualFunction()
{
    // Base implementation
}

/*!
    Regular virtual function implementation.
*/
void BaseDevice::regularVirtualFunction()
{
    // Base implementation
}

/*!
    \internal
*/
void BaseDevice::protectedInternalFunction()
{
    // Base implementation
}

void BaseDevice::privateVirtualFunction()
{
    // Base implementation - private, no docs needed
}

