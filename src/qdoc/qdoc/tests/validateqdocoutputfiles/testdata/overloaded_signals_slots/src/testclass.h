// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTCLASS_H
#define TESTCLASS_H

class String;

class TestClass
{
public:
    TestClass();

    void regularFunction(int value);
    void regularFunction(const String &value);

    // Mock signals (just function declarations for documentation purposes)
    void dataChanged(int value);
    void dataChanged(const String &value);

    // Mock slots (just function declarations for documentation purposes)
    void process(int value);
    void process(const String &value);
};

#endif // TESTCLASS_H

