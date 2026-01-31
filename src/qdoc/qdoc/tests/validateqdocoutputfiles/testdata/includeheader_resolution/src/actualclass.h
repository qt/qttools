// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

/*!
    \class ActualClass
    \inmodule IncludeHeaderModule
    \brief A class without a convenience header.

    This class has no convenience header mapping, so the include
    statement should show the actual header filename: actualclass.h

    This tests QTBUG-86364 fix for non-Qt projects using traditional
    header file naming.
*/
class ActualClass
{
public:
    /*!
        \brief Default constructor.
    */
    ActualClass() = default;

    /*!
        \brief A simple function.
    */
    void doSomething();
};

