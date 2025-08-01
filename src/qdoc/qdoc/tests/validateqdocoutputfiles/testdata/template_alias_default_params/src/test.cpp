// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

#include "test.h"

// This file ensures the template aliases are processed by QDoc
namespace {
    TestAliases::Foo<> foo_instance;
    TestAliases::Bar<int> bar_instance;
    TestAliases::SimpleAlias simple_instance;
}

