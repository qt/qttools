// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

class Target;
class Other;

class Local
{
public:
    Local();
};

bool combine(const Local &local, const Target &target);
bool operator==(const Local &lhs, const Other &rhs);
bool operator!=(const Local &lhs, const Other &rhs);
