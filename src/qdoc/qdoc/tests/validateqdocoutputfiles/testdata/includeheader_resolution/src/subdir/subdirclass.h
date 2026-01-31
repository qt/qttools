// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

/*!
    \class SubdirClass
    \inmodule IncludeHeaderModule
    \brief A class in a subdirectory.

    This class is located in a subdirectory (subdir/subdirclass.h),
    so the include statement should preserve the subdirectory path.

    This tests that QTBUG-86364 fix correctly computes include-relative
    paths for headers in subdirectories.
*/
class SubdirClass
{
public:
    /*!
        \brief Default constructor.
    */
    SubdirClass() = default;

    /*!
        \brief A simple function.
    */
    void doSubdirThing();
};

