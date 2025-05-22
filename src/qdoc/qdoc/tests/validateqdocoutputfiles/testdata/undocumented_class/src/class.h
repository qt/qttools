// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

#pragma once

void globalFunc() {}

class TestClass
{
public:
    TestClass() = default;
    void method();

    int i {0};
};
