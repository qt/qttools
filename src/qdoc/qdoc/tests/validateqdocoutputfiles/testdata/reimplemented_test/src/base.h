// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BASE_H
#define BASE_H

class BackendClass
{
public:
    BackendClass() {}
    virtual ~BackendClass() {}

    virtual void implementMe() {}

    virtual void overrideMe() {}
};

class IntermediateClass : public BackendClass
{
public:
    IntermediateClass() {}

    void implementMe() override {}
};

class BaseClass
{
public:
    BaseClass() {}
    virtual ~BaseClass() {}

    virtual void internalFunction() {}

    virtual void publicFunction() {}
};

#endif // BASE_H

