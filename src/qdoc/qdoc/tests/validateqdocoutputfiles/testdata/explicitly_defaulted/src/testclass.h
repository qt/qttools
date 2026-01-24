// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef EXPLICITLYDEFAULTEDTEST_H
#define EXPLICITLYDEFAULTEDTEST_H

namespace QDocTests {

class ExplicitlyDefaultedTest {
public:
    ExplicitlyDefaultedTest() = default;
    ExplicitlyDefaultedTest(const ExplicitlyDefaultedTest &other) = default;
    ExplicitlyDefaultedTest(ExplicitlyDefaultedTest &&other) = default;
    ~ExplicitlyDefaultedTest() = default;
    ExplicitlyDefaultedTest &operator=(const ExplicitlyDefaultedTest &other) = default;
    ExplicitlyDefaultedTest &operator=(ExplicitlyDefaultedTest &&other) = default;

private:
    int m_value{0};
};

class UnnamedParameterTest {
public:
    UnnamedParameterTest() = default;
    UnnamedParameterTest &operator=(const UnnamedParameterTest &) = default;
    UnnamedParameterTest &operator=(UnnamedParameterTest &&) = default;

private:
    int m_value{0};
};

} // QDocTests

#endif // EXPLICITLYDEFAULTEDTEST_H

