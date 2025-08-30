// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module CompletelyHidden
    \brief Nothing to see here.
*/

/*!
    \class HiddenFriendsClass
    \inmodule CompletelyHidden
    \brief A test class demonstrating hidden friend functions.

    This class contains hidden friend functions declared in the private section.
    These should appear in documentation regardless of includeprivate setting
    because they have public visibility through ADL.
*/
class HiddenFriendsClass
{
public:
    /*!
        \brief Public constructor.

        Creates a HiddenFriendsClass instance.
    */
    HiddenFriendsClass();

    /*!
        \brief Public member function.

        This should always appear in documentation.
    */
    void publicMethod();

private:
    /*!
        \brief Private member function.

        This should NOT appear when includeprivate is false.
    */
    void privateMethod();

    /*!
        \brief Hidden friend equality operator.

        This hidden friend should appear in documentation despite being
        declared in the private section, because it has public visibility
        through argument-dependent lookup (ADL).

        \a lhs Left-hand side operand.
        \a rhs Right-hand side operand.

        Returns \c true if objects are equal.
    */
    friend bool operator==(const HiddenFriendsClass &lhs, const HiddenFriendsClass &rhs);

    /*!
        \brief Hidden friend swap function.

        This hidden friend should also appear in documentation.
        Uses the standard swap idiom.

        \a a First object to swap.
        \a b Second object to swap.
    */
    friend void swap(HiddenFriendsClass &a, HiddenFriendsClass &b)
    {
        // Implementation would go here
    }

    int m_privateData = 42; // Undocumented private member - should not appear
};

