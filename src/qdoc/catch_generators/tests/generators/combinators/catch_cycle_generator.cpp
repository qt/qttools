// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_generators/namespaces.h>
#include <catch_generators/generators/combinators/cycle_generator.h>

#include <catch/catch.hpp>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;

// REMARK: We use fixed-values-generators for those tests so that it
// is trivial to identify when their generation will end, which
// values we should expect and how many values we should expect.
// This is unfortunately not general, but we don't have, by default,
// enough tools to generalize this without having to provide our own
// (being able to generate fixed values from a vector) and adding more
// to the complexity, which is already high.

TEST_CASE(
    "The xn + m element, where 0 < m < n, from a repeating generator whose underlying generator produces n elements, will produce an element equivalent to the mth element of the generation produced by the underlying generator",
    "[Cycle][Combinators]"
) {
    std::size_t n{10};

    auto owned_generator{Catch::Generators::values({'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'})};
    auto owned_generator_copy{Catch::Generators::values({'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'})};

    auto original_generation = GENERATE_REF(take(1, chunk(n, std::move(owned_generator_copy))));

    std::size_t x = GENERATE(take(10, random(std::size_t{0}, std::size_t{20})));
    std::size_t m = GENERATE_COPY(take(10, random(std::size_t{1}, std::size_t{n})));

    auto repeating_generator = cycle(std::move(owned_generator));
    auto repeating_generation = GENERATE_REF(take(1, chunk((x * n) + m, std::move(repeating_generator))));

    REQUIRE(repeating_generation.back() == original_generation[m - 1]);
}

SCENARIO("Repeating a generation ad infinitum", "[Cycle][Combinators]") {
    GIVEN("Some finite generator") {
        std::size_t values_amount{3};

        auto owned_generator{Catch::Generators::values({'a', 'b', 'c'})};
        auto owned_generator_copy{Catch::Generators::values({'a', 'b', 'c'})};

        AND_GIVEN("A way to repeat the generation of that generator infinitely") {
            auto repeating_generator = cycle(std::move(owned_generator));

            WHEN("Generating exactly enough values to exhaust the original generator") {
                auto repeating_generation = GENERATE_REF(take(1, chunk(values_amount, std::move(repeating_generator))));
                auto original_generation = GENERATE_REF(take(1, chunk(values_amount, std::move(owned_generator_copy))));

                THEN("The repeating generator behaves equally to the original finite generator") {
                    REQUIRE(repeating_generation == original_generation);
                }
            }

            WHEN("Generating exactly n times the amount of values required to exhaust the original generator") {
                std::size_t n = GENERATE(take(10, random(2, 10)));

                auto original_generation = GENERATE_REF(take(1, chunk(values_amount, std::move(owned_generator_copy))));
                auto repeating_generation = GENERATE_REF(take(n, chunk(values_amount, std::move(repeating_generator))));

                THEN("The n generation of the repeating generator are always the same as the generation of the original generation") {
                    REQUIRE(repeating_generation == original_generation);
                }
            }
        }
    }
}
