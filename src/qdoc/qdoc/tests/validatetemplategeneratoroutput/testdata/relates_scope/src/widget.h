// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

class Widget
{
public:
    Widget();
};

namespace Ns {
enum class TimerId { Invalid };
enum Mode { Fast, Slow };
using Handle = int;
Handle makeHandle(int id);
Handle makeHandle(const char *name);

class Gadget
{
public:
    enum Level { Low, High };
    enum class Grade { A, B };
};
}

enum class GlobalId { None };
