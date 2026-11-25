// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

struct Name {};

class Probe
{
public:
    void setText(const char *text);
    void setName(const Name &name);
    void setNames(const Name &&name);
    void fill(const int values[4]);
};
