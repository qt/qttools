// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

// dummy declaration
namespace std {
    template<class T1, class T2> struct pair;
}

template <class T1, class T2>
using StdPair = std::pair<T1, T2>;
