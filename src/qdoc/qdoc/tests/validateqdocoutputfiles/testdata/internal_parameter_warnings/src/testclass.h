// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module InternalParameterModule
    \brief Test module for internal parameter warnings.
*/

/*!
    \class TestClass
    \inmodule InternalParameterModule
    \brief Test class demonstrating internal parameter warning behavior.

    This class contains functions with both regular and internal documentation
    to verify that parameter validation warnings are properly suppressed for
    internal documentation when showinternal is disabled.
*/
class TestClass
{
public:
    /*!
        \internal
        \brief Internal function with parameter typo.

        This function takes \a wrongParam (typo: should be correctParam).
        This should NOT generate a warning when showinternal=false.
    */
    void internalFunction(int correctParam);

    /*!
        \brief Regular function with correct parameters.

        This function takes \a correctParam as expected.
        This should not generate any warnings.
    */
    void correctFunction(int correctParam);

    /*!
        \internal
        \brief Internal function with correct parameters.

        This function takes \a correctParam as expected.
        This should not generate any warnings.
    */
    void correctInternalFunction(int correctParam);
};

