// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

// A custom main was provided to avoid linking errors when using minGW
// that were appearing in CI.
// See https://github.com/catchorg/Catch2/issues/1287
int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
