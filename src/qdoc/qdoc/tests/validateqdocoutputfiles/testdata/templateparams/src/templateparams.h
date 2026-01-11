// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#pragma once

// Test case 1: Fully documented template class - no warnings expected
template <typename T, typename Allocator>
class FullyDocumented
{
public:
    void add(T value);
};

// Test case 2: Partially documented template class - warning expected for U
template <typename T, typename U>
class PartiallyDocumented {};

// Test case 3: Template function with both template and regular params documented
template <typename T>
void templateFunc(T value);

// Test case 4: Template class with member function using inherited template param
template <typename Key, typename Value>
class Container
{
public:
    void insert(Key k, Value v);

    template <typename Other>
    void merge(Other source);
};

// Test case 5: Wrong param name documented - warning expected
template <typename T>
class WrongParamDocumented {};
