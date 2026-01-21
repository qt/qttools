// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "autogentest.h"

/*!
    \module AutoDocTests
    \brief The AutoDocTests module contains test classes for auto-generated SMF documentation.
*/

/*!
    \namespace AutoDocTests
    \inmodule AutoDocTests
    \brief The AutoDocTests namespace contains test classes for auto-generated documentation.
*/

/*!
    \class AutoDocTests::AutoGenSmfTest
    \inmodule AutoDocTests
    \brief A test class with explicitly defaulted SMFs and no explicit documentation.

    This class tests that QDoc correctly auto-generates documentation
    for explicitly defaulted special member functions when no \\fn
    documentation is provided.
*/

// NOTE: No \fn documentation for the SMFs - they should get auto-generated docs

/*!
    \class AutoDocTests::DeletedSmfTest
    \inmodule AutoDocTests
    \brief A test class with deleted SMFs and no explicit documentation.

    This class tests that QDoc correctly auto-generates documentation
    for deleted special member functions when no \\fn documentation
    is provided.
*/

// NOTE: No \fn documentation for the deleted SMFs - they should get auto-generated docs

/*!
    \class AutoDocTests::VirtualDtorTest
    \inmodule AutoDocTests
    \brief A test class with a virtual destructor.

    This class tests that QDoc correctly notes when a destructor is virtual
    in the auto-generated documentation.
*/

// NOTE: No \fn documentation - auto-generated docs should note the virtual destructor

