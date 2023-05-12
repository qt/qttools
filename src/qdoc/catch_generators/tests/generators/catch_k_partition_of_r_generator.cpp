// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_generators/namespaces.h>
#include <catch_generators/generators/k_partition_of_r_generator.h>

#include <catch/catch.hpp>

#include <numeric>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;

SCENARIO("Generating a k-partition of a real number", "[Partition][Reals]") {
    GIVEN("A real number r greater or equal to zero") {
        double r = GENERATE(take(10, random(0.0, 1000000.0)));

        AND_GIVEN("An amount of desired elements k greater than zero") {
            std::size_t k = GENERATE(take(10, random(1, 100)));

            WHEN("A k-partition of r is generated") {
                auto k_partition = GENERATE_COPY(take(10, k_partition_of_r(r, k)));

                THEN("The partition contains k elements") {
                    REQUIRE(k_partition.size() == k);

                    AND_THEN("The sum of those elements is r") {
                        REQUIRE(std::accumulate(k_partition.begin(), k_partition.end(), 0.0) == Approx(r));
                    }
                }
            }
        }
    }
}

TEST_CASE("All 1-partition of r are singleton collection with r as their element", "[Partition][Reals][SpecialCase]") {
    double r = GENERATE(take(10, random(0.0, 1000000.0)));
    auto k_partition = GENERATE_COPY(take(10, k_partition_of_r(r, 1)));

    REQUIRE(k_partition.size() == 1);
    REQUIRE(k_partition.front() == r);
}
