// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module PrivateHeadersModule
    \brief Test module for private header processing.
*/

/*!
    \class PublicClass
    \inmodule PrivateHeadersModule
    \brief A public class that should always be documented.

    This class is in the public header and should always appear
    in documentation regardless of showinternal setting.
*/
class PublicClass
{
public:
    /*!
        \brief Public constructor.

        This should always appear in documentation.
    */
    PublicClass();

    /*!
        \brief Public function.

        This should always appear in documentation.
    */
    void publicFunction();

    /*!
        \internal
        \brief Internal public function.

        This should only appear when showinternal is enabled.
    */
    void internalPublicFunction();

private:
    class PrivateData; // Forward declaration to private implementation
    PrivateData *d;
};

