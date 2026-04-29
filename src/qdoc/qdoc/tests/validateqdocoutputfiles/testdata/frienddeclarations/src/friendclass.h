// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module FriendModule
    \brief Test module for friend declarations.
*/

/*!
    \class FriendClass
    \inmodule FriendModule
    \brief A class with friend declaration and hidden friend.

    This class has both a friend declaration (without inline definition)
    and a hidden friend (with inline definition).

    The friend declaration for operator!= should NOT create a documentation
    node under the class - only the hidden friend operator== should appear.
*/
class FriendClass
{
public:
    /*!
        \brief Default constructor.
    */
    FriendClass() = default;

    /*!
        \brief Public member function.
    */
    void publicMethod();

private:
    // friend declaration with a sibling namespace-scope redeclaration
    friend bool operator!=(const FriendClass &lhs, const FriendClass &rhs);

    /*!
        \brief Hidden friend equality operator.

        This hidden friend (defined inline) should appear in documentation.
        \a lhs Left-hand side operand.
        \a rhs Right-hand side operand.
        Returns \c true if objects are equal.
    */
    friend bool operator==(const FriendClass &lhs, const FriendClass &rhs)
    {
        (void)lhs;
        (void)rhs;
        return true;
    }

    int m_data = 0;
};

