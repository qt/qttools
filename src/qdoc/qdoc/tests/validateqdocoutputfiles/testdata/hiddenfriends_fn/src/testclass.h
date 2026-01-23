// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

class TestClass
{
public:
    TestClass() = default;

    /*!
        \brief Public member function.
    */
    void publicMethod();

private:
    // Hidden friend:
    // Documentation is in testclass.cpp using unqualified \fn syntax
    friend bool operator==(const TestClass &lhs, const TestClass &rhs)
    {
        (void)lhs;
        (void)rhs;
        return true;
    }

    // Hidden friend:
    // Documentation is in testclass.cpp using unqualified \fn syntax
    friend bool operator<(const TestClass &lhs, const TestClass &rhs)
    {
        (void)lhs;
        (void)rhs;
        return false;
    }

    int m_data = 0;
};

