// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch/catch.hpp>

#include <catch_generators/namespaces.h>
#include <catch_generators/utilities/semantics/generator_handler.h>

using namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE;

TEST_CASE(
    "Calling next 0 < n times and then calling get on a GeneratorHandler wrapping a generator behaves the same as only calling next (n-1) times and then get on the generator that is wrapped",
    "[GeneratorHandler][Utilities][Semantics][Generators]"
) {
    auto n = GENERATE(take(100, random(1, 100)));
    auto generator_values = GENERATE_COPY(take(1, chunk(n, random(0, 100000))));

    auto generator_handler = handler(Catch::Generators::from_range(generator_values.begin(), generator_values.end()));
    auto generator{Catch::Generators::from_range(generator_values.begin(), generator_values.end())};

    generator_handler.next();
    for (int times{1}; times < n; ++times) {
        generator_handler.next();
        generator.next();
    }

    REQUIRE(generator_handler.get() == generator.get());
}
