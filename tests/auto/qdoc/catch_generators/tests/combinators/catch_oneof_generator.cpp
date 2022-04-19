/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <std_catch_conversions.hpp>

#include "namespaces.hpp"
#include "generators/k_partition_of_r_generator.hpp"
#include "generators/combinators/oneof_generator.hpp"

#include <catch.hpp>

#include <statistics/percentages.hpp>

#include <cmath>
#include <iterator>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;
using namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE;

template<typename T>
std::remove_reference_t<T> copy_value(T value) { return value; }

template<typename T>
using Histogram = std::unordered_map<T, std::size_t>;

template<typename InputIt, typename GroupBy>
auto make_histogram(InputIt begin, InputIt end, GroupBy&& group_by) {
    Histogram<std::invoke_result_t<GroupBy, decltype(*begin)>> histogram{};

    while (begin != end) {
        auto key{std::invoke(std::forward<GroupBy>(group_by), *begin)};

        histogram.try_emplace(key, 0);
        histogram[key]+= 1;
        ++begin;
    }

    return histogram;
}

template<typename T>
struct DistributionError {
    T value;
    double probability;
    double expected_probability;
};

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const DistributionError<T>& error) {
    return os << "DistributionError{" <<
           "The value { " << error.value <<
           " } appear with a probability of { " << error.probability <<
           " } while a probability of { " << error.expected_probability << " } was expected." <<
           "}";
}

// REMARK: The following should really return an Either of unit/error
// but std::variant in C++ is both extremely unusable and comes with a
// strong overhead unless certain conditions are met.
// For this reason, we keep to the less intutitive optional error.

/*!
 * Returns true when the given \a sequence approximately respects a
 * given distribution.
 *
 * The \a sequence respects a given distribution when the count of
 * each collection of values is a percentage of the total values that
 * is near the percentage probability described by distribution.
 *
 * The values in \a sequence are collected according to \a group_by.
 * \a group_by, given an element of \a sequence, should return a value
 * of some type that represent the category of the inspected value.
 * Values that have the same category share their count.
 *
 * The distribution that should be respected is given by \a
 * probability_of. \a probability_of is a function that takes a
 * category that was produced from a call to \a group_by and returns
 * the expect probability, in percentage, of apperance for that
 * category.
 *
 * The given probability is then compared to the one found by counting
 * the element of \a sequence under \a group_by, to ensure that it
 * matches.
 *
 * The margin of error for the comparison is given, in percentage
 * points, by \a margin.
 * The approximation uses an absolute comparison and scales the
 * margin inversely based on the size of \a sequence, to account for the
 * precision of the data set itself.
 *
 * When the distribution is not respected, a DistributionError is
 * returned enclosed in an optional value.
 * The error allows reports which the first category for which the
 * comparison failed, along with its expected probability and the one
 * that was actually inferred from \a sequence.
 */
template<typename T, typename GroupBy, typename ProbabilityOf>
std::optional<DistributionError<T>> respects_distribution(std::vector<T>&& sequence, GroupBy&& group_by, ProbabilityOf&& probability_of, double margin = 33) {
    std::size_t data_point_amount{sequence.size()};

    // REMARK: We scale the margin based on the data set to allow for
    // an easier change in downstream tests.
    // The precision required for the approximation will vary
    // depending on how many values we generate.
    // The amount of values we generate depends on how much time we
    // want the tests to take.
    // This amount may change in the future. For example, as code is
    // added and tests are added, we might need some expensive
    // computations here and there.
    // Sometimes, this will increase the test suite runtime without an
    // obvious way of improving the performance of the underlying code
    // to reduce it.
    // In those cases, the total run time can be decreased by running
    // less generations for battle-tested tests.
    // If some code has not been changed for a long time, it will have
    // had thousands of generations by that point, giving us a good
    // degree of certainty of it not being bugged (for whatever bugs
    // the tests account for).
    // Then, running a certain amount of generation is not required
    // anymore such that some of them can be optimized out.
    // For tests like the one using this function, where our ability
    // to test is always dependent on the amount of generations,
    // changing the generated amount will mean that we will need to
    // change our conditions too, potentially changing the meaning of
    // the test.
    // To take this into account, we perform a scaling on the
    // condition itself, so that if the amount of data points that are
    // generated changes, we do not generally have to change anything
    // in the condition.
    //
    // For this case, we scale logarithmically_10 for the simple
    // reason that we tend to generate values in power of tens,
    // starting with the 100 values default that Quickcheck used.
    //
    // The default value for the margin on which the scaling is based,
    // was chosen heuristically.
    // As we expect generation under 10^3 to be generally meaningless
    // for this kind of testing, the value was chosen so that it would
    // start to normalize around that amount.
    // Deviation of about 5-10% were identified trough various
    // generations for an amount of data points near 1000, while a
    // deviation of about 1-3% was identified with about 10000 values.
    // With the chosen default value, the scaling approaches those
    // percentage points with some margin of error.
    //
    // We expect up to a 10%, or a bit more, deviation to be suitable
    // for our purposes, as it would still allow for a varied
    // distribution in downstream consumers.
    double scaled_margin{margin * (1.0/std::log10(data_point_amount))};

    auto histogram{make_histogram(sequence.begin(), sequence.end(), std::forward<GroupBy>(group_by))};

    for (auto& bin : histogram) {
        auto [key, count] = bin;

        double actual_percentage{percent_of(static_cast<double>(count), static_cast<double>(data_point_amount))};
        double expected_percentage{std::invoke(std::forward<ProbabilityOf>(probability_of), key)};

        if (!(actual_percentage == Approx(expected_percentage).margin(scaled_margin)))
            return std::make_optional(DistributionError<T>{key, actual_percentage, expected_percentage});
    }

    return std::nullopt;
}

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

// REMARK: This tests cannot be precise as it depends on randomized
// output.
// For this reason, we mark it as !mayfail.
// This allows us to see cases where it fails without having the
// test-run itself fail.
// We generally expect this test to not fail, but it may fail randomly
// every now and then simply because of how a correctly randomized
// distrubution may behave.
// As long as this test doesn't fail consistently, with values that
// shows an unsustainable deviation, it should be considered to be
// working.
SCENARIO("Observing the distribution of generators that are chosen from", "[OneOf][Combinators][Statitics][!mayfail]") {
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

        AND_GIVEN("A probability of being chosen, in percentage, for each of the generators, such that the sum of the percentages is an hundred") {
            std::vector<double> probabilities = GENERATE_COPY(take(10, k_partition_of_r(100.0, generators_amount)));

            AND_GIVEN("A choosing generator for those generators based on the given probabilities") {
                auto choosing_generator = oneof(std::move(generators), probabilities);

                WHEN("A certain amount of values are generated from the choosing generator") {
                    auto values = GENERATE_REF(take(1, chunk(10000, std::move(choosing_generator))));

                    THEN("The distribution of elements for each generator approximately respect the weight that was given to it") {
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

                THEN("The distribution of elements approximately tends to being uniform over the generators") {
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

                THEN("The distribution of elements approximately tends to being uniform for each value") {
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
