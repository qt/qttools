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

/*!
    \since 6.11
    \overload primary

    Test function to validate QTBUG-140510: Functions marked with \\overload primary
    should require parameter documentation just like non-overloaded functions,
    since they serve as the main documentation for their function family.

    This primary overload has a parameter \a param that is documented here,
    demonstrating the correct practice for primary overload documentation.
*/
void TestOverloads::parameterWarningTest(int param)
{
    // Implementation for primary overload with documented parameter
}

/*!
    \since 6.11
    \overload parameterWarningTest()

    This overload correctly does not generate undocumented parameter warnings
    for its parameters, because regular overloads inherit their documentation
    from the primary overload. The parameters are intentionally not documented
    to test that the warning suppression works for regular overloads.
*/
void TestOverloads::parameterWarningTest(const char *message, int level)
{
    // Implementation for overload with undocumented parameters
}

/*!
    \since 6.11
    \overload parameterWarningTest()

    Another overload that correctly does not generate parameter warnings.
    Parameters are intentionally undocumented to test regular overload behavior.
*/
void TestOverloads::parameterWarningTest(const RegularExpression &pattern, bool enabled)
{
    // Implementation for overload with undocumented parameters
}

/*!
    \since 6.11
    \overload primary

    Test function to validate QTBUG-140508: Primary overloads should not
    display "This function overloads..." text. Instead, they should appear
    as the main documentation entry.

    This function demonstrates the intended behavior where the primary overload
    contains the main documentation and does not show overload linking text.
*/
void TestOverloads::linkingTest()
{
    // Implementation for primary overload - no overload text should appear
}

/*!
    \since 6.11
    \overload linkingTest()

    This overload should show "This function overloads linkingTest()."
    and successfully link to the primary overload above.
*/
void TestOverloads::linkingTest(int value)
{
    // Implementation for overload that should link to primary
}

/*!
    \since 6.11
    \overload linkingTest()

    Another overload that should link to the primary linkingTest() function.
*/
void TestOverloads::linkingTest(const char *message)
{
    // Implementation for overload that should link to primary
}

// Test case for multiple classes with same-named methods each marked as \overload primary
// This should NOT generate warnings about "Multiple primary overloads"

/*!
    \class TestDate
    \inmodule TestQDocOverloadCommand
    \brief Test class simulating QDate.

    This class tests whether QDoc incorrectly reports "Multiple primary overloads"
    when different classes have methods with the same name, using shared comment nodes.
*/

/*!
    \overload primary
    \fn void TestDate::toString() const
    \fn void TestDate::toString(int format) const

    Returns the date as a string. The \a format parameter determines the format
    of the result string.

    This uses shared comment nodes (multiple \\fn commands) just like Eddy's
    qtbase change, which triggers the bug.
*/
void TestDate::toString() const
{
}

void TestDate::toString(int format) const
{
}

/*!
    \overload toString()

    Returns the date as a string using the specified \a formatString.
*/
void TestDate::toString(const char *formatString) const
{
}

/*!
    \class TestTime
    \inmodule TestQDocOverloadCommand
    \brief Test class simulating QTime.

    This class tests whether QDoc incorrectly reports "Multiple primary overloads"
    when different classes have methods with the same name, using shared comment nodes.
*/

/*!
    \overload primary
    \fn void TestTime::toString() const
    \fn void TestTime::toString(int format) const

    Returns the time as a string. The \a format parameter determines the format
    of the result string.

    This uses shared comment nodes (multiple \\fn commands) just like Eddy's
    qtbase change, which triggers the bug.
*/
void TestTime::toString() const
{
}

void TestTime::toString(int format) const
{
}

/*!
    \overload toString()

    Returns the time as a string using the specified \a formatString.
*/
void TestTime::toString(const char *formatString) const
{
}

/*!
    \class TestDateTime
    \inmodule TestQDocOverloadCommand
    \brief Test class simulating QDateTime.

    This class tests whether QDoc incorrectly reports "Multiple primary overloads"
    when different classes have methods with the same name, using shared comment nodes.
*/

/*!
    \overload primary
    \fn void TestDateTime::toString() const
    \fn void TestDateTime::toString(int format) const

    Returns the datetime as a string. The \a format parameter determines the format
    of the result string.

    This uses shared comment nodes (multiple \\fn commands) just like Eddy's
    qtbase change, which triggers the bug.
*/
void TestDateTime::toString() const
{
}

void TestDateTime::toString(int format) const
{
}

/*!
    \overload toString()

    Returns the datetime as a string using the specified \a formatString.
*/
void TestDateTime::toString(const char *formatString) const
{
}

/*!
    \class TestPositionDependent
    \inmodule TestQDocOverloadCommand
    \brief Test class to verify position-dependent primary overload behavior.

    This class tests that when using shared comment nodes with \\overload primary,
    the FIRST \\fn command becomes the primary overload, regardless of how the
    functions would naturally sort by signature.
*/

/*!
    \overload primary
    \fn void TestPositionDependent::convert(int value, bool flag) const
    \fn void TestPositionDependent::convert() const

    Converts data. The \a value and \a flag parameters control the conversion.

    NOTE: The \\fn commands are intentionally in reverse sort order.
    The first \\fn has 2 parameters, the second has 0.
    By natural sorting, convert() would come first (fewer parameters).
    But with position-dependent behavior, convert(int, bool) should be the primary
    because it's the FIRST \\fn command in this shared comment block.
*/
void TestPositionDependent::convert(int value, bool flag) const
{
}

void TestPositionDependent::convert() const
{
}

/*!
    \overload convert()

    Converts the specified \a text.

    This overload should link to the PRIMARY convert function, which is
    convert(int, bool) because it was the FIRST \\fn in the shared comment block,
    NOT convert() even though convert() would sort first by signature.
*/
void TestPositionDependent::convert(const char *text) const
{
}

