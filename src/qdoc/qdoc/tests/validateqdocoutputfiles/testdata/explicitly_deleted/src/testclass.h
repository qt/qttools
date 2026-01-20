// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef EXPLICITLYDELETEDTEST_H
#define EXPLICITLYDELETEDTEST_H

namespace QDocTests {

class ExplicitlyDeletedTest {
public:
    ExplicitlyDeletedTest() = default;
    ExplicitlyDeletedTest(const ExplicitlyDeletedTest &other) = delete;
    ExplicitlyDeletedTest(ExplicitlyDeletedTest &&other) = delete;
    ExplicitlyDeletedTest &operator=(const ExplicitlyDeletedTest &other) = delete;
    ExplicitlyDeletedTest &operator=(ExplicitlyDeletedTest &&other) = delete;

private:
    int m_value{0};
};

} // QDocTests

#endif // EXPLICITLYDELETEDTEST_H

