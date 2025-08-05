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
};

#endif // TESTOVERLOADS_H

