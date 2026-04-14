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

// QTBUG-145790: Container class that serves as a \relates target
// for a hidden friend defined in a different class.
class Container
{
};

// Generic template — the customization point.
// Has the same name as the hidden friend in MyClass below.
template <typename T>
bool myEquals(const T &a, const T &b)
{
    return a == b;
}

// Class with a hidden friend overload of myEquals.
// The hidden friend's doc uses \relates Container, not \relates MyClass.
class MyClass
{
public:
    MyClass(int v) : value(v) { }

private:
    int value;
    friend bool myEquals(const MyClass &a, const MyClass &b) noexcept { return a.value == b.value; }
};
