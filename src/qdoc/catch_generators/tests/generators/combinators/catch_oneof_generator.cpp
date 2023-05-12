// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_conversions/std_catch_conversions.h>

#include <catch_generators/namespaces.h>
#include <catch_generators/generators/k_partition_of_r_generator.h>
#include <catch_generators/generators/combinators/oneof_generator.h>
#include <catch_generators/generators/combinators/cycle_generator.h>
#include <catch_generators/utilities/statistics/percentages.h>
#include <catch_generators/utilities/statistics/distribution.h>
#include <catch_generators/utilities/semantics/copy_value.h>

#include <catch/catch.hpp>

#include <cmath>
#include <iterator>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;
using namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE;

SCENARIO("Choosing between one of many generators", "[OneOf][Combinators]") {
    GIVEN("Some generators producing values of the same type") {
        auto generators_amount = GENERATE(take(10, random(1, 10)));
        auto generators_values = GENERATE_COPY(take(10, chunk(generators_amount, random(0, 100000))));

        std::vector<Catch::Generators::GeneratorWrapper<int>> generators;
        generators.reserve(generators_amount);
        std::transform(
            generators_values.begin(), generators_values.end(), std::back_inserter(generators),
            [](auto& value){ return Catch::Generators::value(copy_value(value)); }
        );

        AND_GIVEN("A generator choosing between them based on some distribution") {
            std::vector<double> weights = GENERATE_COPY(take(10, k_partition_of_r(100.0, generators_amount)));
            auto choosing_generator = oneof(std::move(generators), std::move(weights));

            WHEN("A value is extracted from the choosing generator") {
                auto generated_value = GENERATE_REF(take(100, std::move(choosing_generator)));

                THEN("The generated value is a member of one of the original generators") {
                    REQUIRE(std::find(generators_values.cbegin(), generators_values.cend(), generated_value) != generators_values.cend());
                }
            }
        }

        AND_GIVEN("A generator choosing between them with the same probability") {
            auto choosing_generator = uniform_oneof(std::move(generators));

            WHEN("A value is extracted from the choosing generator") {
                auto generated_value = GENERATE_REF(take(100, std::move(choosing_generator)));

                THEN("The generated value is a member of one of the original generators") {
                    REQUIRE(std::find(generators_values.cbegin(), generators_values.cend(), generated_value) != generators_values.cend());
                }
            }
        }

        AND_GIVEN("A generator choosing between them such that each possible value has the same probability of being chosen") {
            auto choosing_generator = uniformly_valued_oneof(std::move(generators), std::vector(generators_amount, std::size_t{1}));

            WHEN("A value is extracted from the choosing generator") {
                auto generated_value = GENERATE_REF(take(100, std::move(choosing_generator)));

                THEN("The generated value is a member of one of the original generators") {
                    REQUIRE(std::find(generators_values.cbegin(), generators_values.cend(), generated_value) != generators_values.cend());
                }
            }
        }
    }
}

// TODO: The following is a generally complex test. Nonetheless, we
// can probably ease some of the complexity by moving it out into some
// generators or by abstracting it a little to remove the need to know
// some of the implementation details.
// Check if this is possible.

// REMARK: [mayfail][distribution]
// This tests cannot be precise as it depends on randomized output.
// For this reason, we mark it as !mayfail.
// This allows us to see cases where it fails without having the
// test-run itself fail.
// We generally expect this test to not fail, but it may fail randomly
// every now and then simply because of how a correctly randomized
// distribution may behave.
// As long as this test doesn't fail consistently, with values that
// shows an unsustainable deviation, it should be considered to be
// working.
SCENARIO("Observing the distribution of generators that are chosen from", "[OneOf][Combinators][Statistics][!mayfail]") {
    GIVEN("Some generators producing values of the same type") {
        std::size_t generators_amount = GENERATE(take(10, random(1, 10)));

        // REMARK: To test the distribution, we want to have some
        // amount of generators to choose from whose generated values
        // can be uniquely reconducted to the generating generator so
        // that we may count how many times a specific generator was
        // chosen.
        // The easiest way would be to have generators that produce a
        // single value.
        // Nonetheless, to test the version that provides an
        // approximate uniform distribution over the values themselves
        // correctly, we need to have generators that can produce a
        // different amount of elements.
        // When that is not the case, indeed, a generator that
        // approximately distributes uniformly over values is
        // equivalent to one that approximately distributes uniformely
        // over the generators themselves.
        // As such, we use ranges of positive integers, as they are
        // the simplest multi-valued finite generator that can be dinamically
        // construted, while still providing an easy way to infer the
        // amount of values it contains so that we can derive the
        // cardinality of our domain.
        // We produce those ranges as disjoint subsequent ranges
        // starting from 0 upward.
        // We require the ranges to be disjoint so that we do not lose
        // the ability of uniquely identifying a generator that
        // produced the value.
        //
        // To do so, we generate a series of disjoint least upper
        // bounds for the ranges.
        // Then, we produce the ith range by using the successor of
        // the (i - 1)th upper bound as its lower bound and the ith
        // upper bound as its upper bound.
        //
        // We take further care to ensure that the collection of upper
        // bounds is sorted, as this simplifies to a linear search our
        // need to index the collection of generators to find the
        // identifying generator and its associated probability.
        std::vector<std::size_t> generators_bounds(generators_amount, 0);
        std::vector<Catch::Generators::GeneratorWrapper<std::size_t>> generators;
        generators.reserve(generators_amount);

        std::size_t lowest_bound{0};
        std::size_t generators_step{1000};
        std::size_t lower_bound_offset{1};

        generators_bounds[0] = Catch::Generators::random(lowest_bound, generators_step).get();
        generators.push_back(Catch::Generators::random(lowest_bound, generators_bounds[0]));

        // We use this one to group together values that are generated
        // from the same generator and to provide an index for that
        // generator to use for finding its associated probability.
        // Since our generators are defined by their upper bounds and
        // the collection of upper bounds is sorted, the first
        // encountered upper bound that is not less than the value
        // itself must be the least upper bound of the generator that
        // produced the value.
        // Then, the index of that upper bound must be the same as the
        // index of the producing generator and its associated
        // probability.
        auto find_index_of_producing_generator = [&generators_bounds](auto value) {
            return static_cast<std::size_t>(std::distance(
                generators_bounds.begin(),
                std::find_if(generators_bounds.begin(), generators_bounds.end(), [&value](auto element){ return value <= element; })
            ));
        };

        for (std::size_t index{1}; index < generators_amount; ++index) {
            generators_bounds[index] = Catch::Generators::random(generators_bounds[index - 1] + lower_bound_offset + 1, generators_bounds[index - 1] + lower_bound_offset + 1 + generators_step).get();
            generators.push_back(Catch::Generators::random(generators_bounds[index - 1] + lower_bound_offset, generators_bounds[index]));
        }

        AND_GIVEN("A probability of being chosen, in percentage, for each of the generators, such that the sum of the percentages is one hundred") {
            std::vector<double> probabilities = GENERATE_COPY(take(10, k_partition_of_r(100.0, generators_amount)));

            AND_GIVEN("A choosing generator for those generators based on the given probabilities") {
                auto choosing_generator = oneof(std::move(generators), probabilities);

                WHEN("A certain amount of values are generated from the choosing generator") {
                    auto values = GENERATE_REF(take(1, chunk(10000, std::move(choosing_generator))));

                    THEN("The distribution of elements for each generator approximately respects the weight that was given to it") {
                        auto maybe_distribution_error{respects_distribution(
                            std::move(values),
                            find_index_of_producing_generator,
                            [&probabilities](auto key){ return probabilities[key]; }
                        )};

                        REQUIRE_FALSE(maybe_distribution_error);
                    }
                }
            }
        }

        AND_GIVEN("A choosing generator for those generators that will choose each generator with the same probability") {
            auto choosing_generator = uniform_oneof(std::move(generators));

            WHEN("A certain amount of values are generated from the choosing generator") {
                auto values = GENERATE_REF(take(1, chunk(10000, std::move(choosing_generator))));

                THEN("The distribution of elements approximates uniformity over the generators") {
                    double probability{uniform_probability(generators_amount)};

                    auto maybe_distribution_error{respects_distribution(
                        std::move(values),
                        find_index_of_producing_generator,
                        [&probability](auto _){ (void)(_); return probability; }
                    )};

                    REQUIRE_FALSE(maybe_distribution_error);
                }
            }
        }

        AND_GIVEN("A choosing generator for those generators that will choose each generator such that each possible value has the same probability of being chosen") {
            // REMARK: We need to know the total amount of
            // unique values that can be generated by our
            // generators, so that we can construct an
            // appropriate distribution.
            // Since our generators are ranges defined by the
            // collection of upper bounds we can find their
            // length by finding the difference between
            // adjacent elements of the collection.
            //
            // Some more care must be taken to ensure tha the
            // correct amount is produced.
            // Since we need our ranges to be disjoint, we
            // apply a small offset from the element of the
            // upper bounds that is used as a lower bound,
            // since that upper bound is inclusive for the
            // range that precedes the one we are making the
            // calculation for.
            //
            // Furthermore, the first range is treated
            // specially.
            // As no range precedes it, it doesn't need any
            // offset to be applied.
            // Additionally, we implicitly use 0 as the first
            // lower bound, such that the length of the first
            // range is indeed equal to its upper bound.
            //
            // To account for this, we remove that offset from
            // the total amount for each range after the first
            // one and use the first upper bound as a seeding
            // value to account for the length of the first
            // range.
            std::vector<std::size_t> generators_cardinality(generators_amount, generators_bounds[0]);

            std::adjacent_difference(generators_bounds.begin(), generators_bounds.end(), generators_bounds.begin());
            std::transform(std::next(generators_cardinality.begin()), generators_cardinality.end(), std::next(generators_cardinality.begin()), [](auto element){ return element - 1; });

            std::size_t output_cardinality{std::accumulate(generators_cardinality.begin(), generators_cardinality.end(), std::size_t{0})};

            auto choosing_generator = uniformly_valued_oneof(std::move(generators), std::move(generators_cardinality));

            WHEN("A certain amount of values are generated from the choosing generator") {
                auto values = GENERATE_REF(take(1, chunk(10000, std::move(choosing_generator))));

                THEN("The distribution of elements approximates uniformity for each value") {
                    double probability{uniform_probability(output_cardinality)};

                    auto maybe_distribution_error{respects_distribution(
                        std::move(values),
                        [](auto value){ return value; },
                        [&probability](auto _){ (void)(_); return probability; }
                    )};

                    REQUIRE_FALSE(maybe_distribution_error);
                }
            }
        }
    }
}

TEST_CASE("A generator with a weight of zero is never chosen when choosing between many generators", "[OneOf][Combinators][SpecialCase]") {
    auto excluded_value = GENERATE(take(100, random(0, 10000)));

    std::vector<Catch::Generators::GeneratorWrapper<int>> generators;
    generators.reserve(2);
    generators.emplace_back(Catch::Generators::random(excluded_value + 1, std::numeric_limits<int>::max()));
    generators.emplace_back(Catch::Generators::value(copy_value(excluded_value)));

    auto generated_value = GENERATE_REF(take(100, oneof(std::move(generators), std::vector{100.0, 0.0})));

    REQUIRE(generated_value != excluded_value);
}

TEST_CASE("The first element of the passed in generators are not lost", "[OneOf][Combinators][GeneratorFirstElement][SpecialCase]") {
    // REMARK: We want to test that, for each generator, the first
    // time it is chosen the first value is produced.
    // This is complicated because of the fact that OneOf chooses
    // random generators in a random order.
    // This means that some generators may never be chosen, never be
    // chosen more than once and so on.
    // Furthermore, this specific test is particularly important only
    // for finite generators or non-completely random, ordered,
    // infinite generators.
    // Additionally, we need to ensure that we test with multiple
    // generators, as this test is a consequence of a first bugged
    // implementation where only the first chosen generator respected
    // the first value, which would pass a test where a single
    // generator is used.
    //
    // This is non-trivial due to the randomized nature of OneOf.
    // It can be simplified if we express it in a non-deterministic
    // way and mark it as mayfail, where we can recognize with a good
    // certainty that the test is actually passing.
    //
    // To avoid having this flaky test, we approach it as follows:
    //
    // We provide some amount of infinite generators. Those generators
    // are ensured to produce one specific value as their first value
    // and then infinitely produce a different value.
    // We ensure that each generator that is provided produces unique
    // values, that is, no two generators produce a first value or 1 <
    // nth value that is equal to the one produced by another
    // generator.
    //
    // Then we pass those generators to oneof and generate enough
    // values such that at least one of the generators must have been
    // chosen twice or more, at random.
    //
    // We count the appearances of each value in the produced set.
    // Then, if a value that is generated by the 1 < nth choice of a
    // specific generator is encountered, we check that the first
    // value that the specific generator would produce is in the set
    // of values that were generated.
    // That is, if a generator has produced his non-first value, it
    // must have been chosen twice or more.
    // This in turn implies that the first time that the generator was
    // chosen, its first value was actually produced.

    struct IncreaseAfterFirst {
        std::size_t increase;
        bool first_application = true;

        std::size_t operator()(std::size_t value) {
            if (first_application) {
                first_application = false;
                return value;
            }

            return value + increase;
        }
    };

    std::size_t maximum_generator_amount{100};
    auto generators_amount = GENERATE_COPY(take(10, random(std::size_t{1}, maximum_generator_amount)));

    std::vector<Catch::Generators::GeneratorWrapper<std::size_t>> generators;
    generators.reserve(generators_amount);

    for (std::size_t index{0}; index < generators_amount; ++index) {
        generators.push_back(Catch::Generators::map(IncreaseAfterFirst{maximum_generator_amount}, cycle(Catch::Generators::value(copy_value(index)))));
    }

    auto values = GENERATE_REF(take(1, chunk(generators_amount + 1, uniform_oneof(std::move(generators)))));
    auto histogram{make_histogram(values.begin(), values.end(), [](auto e){ return e; })};

    for (std::size_t index{0}; index < generators_amount; ++index) {
        std::size_t second_value{index + maximum_generator_amount};
        histogram.try_emplace(second_value, 0);

        if (histogram[second_value] > 0) {
            REQUIRE(histogram.find(index) != histogram.end());
        }
    }
}
