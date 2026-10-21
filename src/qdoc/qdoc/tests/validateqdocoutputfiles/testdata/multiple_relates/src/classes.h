// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

class Alpha
{
public:
    Alpha();
};

class Beta
{
public:
    Beta();
};

class Gamma
{
public:
    Gamma();
};

bool convert(const Alpha &from, Beta &to);
bool operator==(const Alpha &lhs, const Beta &rhs);
bool operator!=(const Alpha &lhs, const Beta &rhs);
void reset(Gamma &gamma);
