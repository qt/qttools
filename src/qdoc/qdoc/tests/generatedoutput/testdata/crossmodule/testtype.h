// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#pragma once

#include "../testcpp/testcpp.h"

namespace CrossModuleRef {
    void documentMeToo();
}

class TestType : public TestQDoc::TestDerived
{
public:
    TestType() {}
    void nothing() {}
};
