// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

template <char Value> struct CharacterTag {};
struct QDocT0 {};
template <typename V> using ConstPointerAlias = const V *;
template <auto Value> struct ValueTag {};

template <typename T>
class PlaceholderProbe
{
public:
    friend void placeholderCheck(PlaceholderProbe &, T &) noexcept {}
};

template <typename T>
class LiteralProbe
{
public:
    friend void literalCheck(LiteralProbe &, CharacterTag<'T'> &) noexcept {}
};

template <typename T>
class ShapeProbe
{
public:
    friend void cvCheck(ShapeProbe &, ConstPointerAlias<T> &) noexcept {}
    friend void refCheck(ShapeProbe &, T &&) noexcept {}
    friend void valueTypeCheck(ShapeProbe &, ValueTag<1> &) noexcept {}
    friend void structureCheck(ShapeProbe &, CharacterTag<'T'> &) noexcept {}
};

template <typename T, int N>
class UnsupportedProbe
{
public:
    friend void expressionCheck(UnsupportedProbe &, ValueTag<N + 1> &) noexcept {}
};
