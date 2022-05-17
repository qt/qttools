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
