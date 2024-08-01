// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTTOOLS_NUMBERS_H
#define QTTOOLS_NUMBERS_H

namespace QDocTests {
class Numbers
{
    public:
    auto add(int a, int b);
    template<typename T>
    T add(T a, T b) { return a + b; }
    template<typename ThisIsMyReallyLongAndComplicatedTemplateName>
    ThisIsMyReallyLongAndComplicatedTemplateName adder(ThisIsMyReallyLongAndComplicatedTemplateName a, int b) { return a + b; }
    int subtract(int a, int b) { return a - b; }

    auto number() const { return m_number; }
    void setNumber(int number) { m_number = number; }

private:
    int m_number;
};

} // namespace QDocTests

#endif //QTTOOLS_NUMBERS_H
