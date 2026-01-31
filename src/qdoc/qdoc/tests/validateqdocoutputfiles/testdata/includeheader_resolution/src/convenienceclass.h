// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

/*!
    \class ConvenienceClass
    \inmodule IncludeHeaderModule
    \brief A class with a convenience header.

    This class has a convenience header mapping (ConvenienceClass),
    so the include statement should show the convenience header name
    instead of the actual filename.

    This tests that Qt-style projects continue to show convenience
    headers when available.
*/
class ConvenienceClass
{
public:
    /*!
        \brief Default constructor.
    */
    ConvenienceClass() = default;

    /*!
        \brief A simple function.
    */
    void doSomethingConvenient();
};

