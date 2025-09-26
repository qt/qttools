// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

#ifndef TESTOVERLOADS_H
#define TESTOVERLOADS_H

class RegularExpression
{
};

class TestOverloads
{
public:
    static void failOnWarning();
    static void failOnWarning(const char *message);
    static void failOnWarning(const RegularExpression &messagePattern);

    static void testFullyQualified();

    static void primary();
    static void primary(int value);
    static void primary(const char *message);

    // Additional test cases for QTBUG-140510: parameter warning issue
    static void parameterWarningTest(int param);
    static void parameterWarningTest(const char *message, int level);
    static void parameterWarningTest(const RegularExpression &pattern, bool enabled);

    // Additional test cases for QTBUG-140508: documentation linking issue
    static void linkingTest();
    static void linkingTest(int value);
    static void linkingTest(const char *message);
};

#endif // TESTOVERLOADS_H

