// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

template <typename T>
class Foo {
public:
    Foo() {}
private:
    T t;
};

template <typename T, typename D>
class Bar {
public:
    Bar() {}
private:
    T t;
    D d;
};

template<template<typename> class X, typename Y>
struct Baz
{
    X<Y> z;
    Baz() : z() {}
};

// Test case: nested template inside class template
// Inner should be able to reference outer's T without requiring it to be documented
template<typename T>
class Outer
{
public:
    template<typename U>
    class Inner
    {
    public:
        T outer_value;
        U inner_value;
    };
};
