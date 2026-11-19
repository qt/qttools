// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

template <char Value> struct CharacterTag {};
template <typename V> using RefAlias = V &;
template <typename V> using ConstPointerAlias = const V *;
template <int Value> struct IntegerTag {};

template <typename T>
class AliasProbe
{
public:
    friend void aliasCheck(AliasProbe &, RefAlias<T>) noexcept {}
};

template <typename T>
class AliasControl
{
public:
    friend void aliasControlCheck(AliasControl &, RefAlias<T>) noexcept {}
};

template <typename T>
class LiteralControl
{
public:
    friend void literalControlCheck(LiteralControl &, CharacterTag<'T'> &) noexcept {}
};

template <typename T, int N>
class StructureControl
{
public:
    friend void structureCheck(StructureControl &, ConstPointerAlias<T> &, T &&,
                               IntegerTag<N> &, IntegerTag<42> &) noexcept {}
};

template <typename T> class PrecedenceProbe;
template <typename T> void precedenceCheck(PrecedenceProbe<T> &) noexcept;
template <typename T> void precedenceValueCheck(PrecedenceProbe<T> &, T) noexcept;

template <typename T>
class PrecedenceProbe
{
public:
    friend void precedenceCheck(PrecedenceProbe &) noexcept {}
    friend void precedenceValueCheck(PrecedenceProbe &, T) noexcept {}
};

template <typename T>
class DedupControl
{
public:
    friend void dedupCheck(DedupControl &, T &);
    friend void dedupCheck(DedupControl &, T &);
};

template <typename T, T N>
class DependentValueProbe
{
public:
    friend void dependentValueCheck(DependentValueProbe &, T) noexcept {}
};
