// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef WIDGET_H
#define WIDGET_H

class Widget
{
public:
    int color;

    int getColor() const;
    void setColor(int c);
};

class Button : public Widget
{
public:
    void click();
};

#endif

