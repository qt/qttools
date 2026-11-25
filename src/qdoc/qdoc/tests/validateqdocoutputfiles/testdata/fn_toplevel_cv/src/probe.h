// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

using real = double;
using ConstReal = const double;

class Probe
{
public:
    void sizeChanged(const real size);
    void setCount(int count);
    void setLevel(volatile int level);
    void setTarget(int *const target);
    void setLimit(ConstReal limit);
    void resize(const int size);
    void resize(const int *size);
    void point(int *position);
    void point(const int *position);
    template <typename T> void store(const T value);
};
