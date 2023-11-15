// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

class Globals {};
inline int foo(int a) { return a; }
inline int foo(int a, bool b) { return b ? a : -a; }
// Undocumented overload
inline int foo(int a, bool b, bool c) { return b || c ? a : -a; }
