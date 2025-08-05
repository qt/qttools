// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

#include "testoverloads.h"

/*!
    \module TestQDocOverloadCommand
    \brief A test module.
*/

/*!
    \class TestOverloads
    \inmodule TestQDocOverloadCommand
    \brief Demonstrates multiple overloads and the \\overload primary feature.

    This test case demonstrates various scenarios for the \\overload command:

    \list 1
        \li Original issue: Using \\overload with unqualified function names
            (e.g., \\overload failOnWarning()) creates dead links when there are
            multiple overloads and QDoc cannot determine which overload should
            be the primary target.
        \li Solution: The \\overload primary command allows explicitly
            designating which overload should be the primary target for
            unqualified \\overload references.
        \li Corner case: Functions named "primary" to test whether the function
            name conflicts with the "primary" keyword in \\overload commands.
    \endlist

    The failOnWarning() functions demonstrate the original problem and its
    solution, while the primary() functions test the corner case where a
    function happens to be named "primary".
*/

/*!
    \since 6.11
    \overload failOnWarning()

    This should link to the primary failOnWarning function, but creates dead
    links when there are multiple overloads and no clear primary function.

    Issue: QDoc cannot resolve which of the three failOnWarning overloads this
    should link to.
*/
void TestOverloads::failOnWarning()
{
    // Implementation for parameterless version
}

/*!
    \since 6.11
    \overload failOnWarning()

    Another overload using the same unqualified \\overload target.
    This also creates dead links for the same reason as above.
*/
void TestOverloads::failOnWarning(const char *message)
{
    // Implementation for const char* version
}

/*!
    \since 6.11
    \overload primary

    Appends a test failure to the test log for each warning that matches
    \\a messagePattern.

    This is explicitly marked as the primary overload with \\overload primary.
    Other overloads should now link to this function when using unqualified
    \\overload commands.
*/
void TestOverloads::failOnWarning(const RegularExpression &messagePattern)
{
    // Implementation for RegularExpression version
}

/*!
    \overload failOnWarning(const RegularExpression &)

    This uses a fully qualified \\overload target to demonstrate the current
    workaround mentioned in the bug report. This should generate a working link.
*/
void TestOverloads::testFullyQualified()
{
    // Test function to demonstrate fully qualified overload reference
}

/*!
    \overload primary

    Corner case test: This function is named "primary" and is marked as the
    primary overload. This tests whether the word "primary" as a function name
    conflicts with the "primary" keyword in the \\overload command.

    This should work correctly because the "primary" keyword is parsed in the
    context of the \\overload command, not as a function name reference.
*/
void TestOverloads::primary()
{
    // Implementation for parameterless primary function
}

/*!
    \overload primary()

    Corner case test: This should link to the primary() function above,
    demonstrating that \\overload primary() correctly references the function
    named "primary" rather than being confused with the "primary" keyword.

    This tests the scenario where a function name happens to be "primary".
*/
void TestOverloads::primary(int value)
{
    // Implementation for primary function with int parameter
}

/*!
    \overload primary()

    Another overload of the function named "primary". This should also link
    to the parameterless primary() function, testing that multiple overloads
    of a function named "primary" work correctly with the \\overload system.
*/
void TestOverloads::primary(const char *message)
{
    // Implementation for primary function with const char* parameter
}

