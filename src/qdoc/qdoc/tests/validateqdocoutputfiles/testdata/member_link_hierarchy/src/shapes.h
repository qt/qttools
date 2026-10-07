// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SHAPES_H
#define SHAPES_H

class Base
{
public:
    int color() const;
};

class Derived : public Base
{
public:
    enum ReportMode { MemberLinks, Warning };

    void paint();
    void sibling();
    void fill();
    void stroke();
};

#endif
