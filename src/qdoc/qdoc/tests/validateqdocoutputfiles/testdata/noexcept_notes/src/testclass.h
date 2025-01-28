// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/*
    Tests that qdoc generates a standard note in the correct format for the
    noexcept operator using the expression, \c some_expression_v.
*/

class TestClass
{
    static constexpr bool some_expression_v = true;

public:
    TestClass();
    void fn() noexcept(some_expression_v);
};
