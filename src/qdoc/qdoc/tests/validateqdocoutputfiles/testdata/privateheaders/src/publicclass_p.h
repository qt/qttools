// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "publicclass.h"

/*!
    \internal
    \class PublicClass::PrivateData
    \inmodule PrivateHeadersModule
    \brief Private implementation class for PublicClass.

    This class is in a private header (_p.h) and contains internal
    implementation details. It should only appear in documentation
    when showinternal is enabled AND private headers are being processed.
*/
class PublicClass::PrivateData
{
public:
    /*!
        \internal
        \brief Constructor for private data.

        This should only appear when showinternal is enabled.
    */
    PrivateData();

    /*!
        \internal
        \brief Internal helper function.

        This function handles internal implementation details.
    */
    void internalHelper();

    /*!
        \internal
        \brief Internal data member.

        Stores internal state for the public class.
    */
    int internalData = 0;

    /*!
        \brief Regular private member.

        This is a regular (non-internal) private member in a private header.
        It should appear when showinternal is enabled, regardless of
        the \internal marking.
    */
    bool regularPrivateMember = false;
};

/*!
    \internal
    \class PrivateOnlyClass
    \inmodule PrivateHeadersModule
    \brief A class that exists only in private headers.

    This entire class is defined in a private header and marked internal.
    It should only appear when both showinternal is enabled AND
    private headers are being processed by QDoc.
*/
class PrivateOnlyClass
{
public:
    /*!
        \internal
        \brief Constructor of private-only class.
    */
    PrivateOnlyClass();

    /*!
        \internal
        \brief Internal method in private class.
    */
    void privateClassMethod();
};

/*!
    \internal
    \brief Internal global function in private header.
    \relates PrivateHeadersModule

    This function exists only in the private header and should only
    appear when showinternal is enabled and private headers are processed.
*/
void internalPrivateHeaderFunction();

