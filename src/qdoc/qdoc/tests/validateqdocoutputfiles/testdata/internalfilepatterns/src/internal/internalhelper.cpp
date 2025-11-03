// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "internalhelper.h"

/*!
    \module TestModule
    \title Test Module
    \brief A test module for internal file patterns.
*/

/*!
    \class InternalHelper
    \inmodule TestModule
    \brief A helper class in the internal directory.

    This class should be marked as Internal because it's in
    a directory matching the *\/internal/\*.h pattern.
*/

InternalHelper::InternalHelper()
{
}

void InternalHelper::doSomething()
{
}

