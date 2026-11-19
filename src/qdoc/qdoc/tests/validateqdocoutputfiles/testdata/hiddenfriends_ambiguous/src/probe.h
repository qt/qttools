// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

template <typename T> class AmbiguousFirst;
template <typename U> class AmbiguousSecond;

template <typename T>
class AmbiguousFirst
{
public:
    friend void ambiguousCheck(AmbiguousFirst<T> &, AmbiguousSecond<T> &, T);
};

template <typename U>
class AmbiguousSecond
{
public:
    friend void ambiguousCheck(AmbiguousFirst<U> &, AmbiguousSecond<U> &, U);
};
