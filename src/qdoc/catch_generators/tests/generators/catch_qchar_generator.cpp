// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_generators/namespaces.h>
#include <catch_generators/generators/qchar_generator.h>

#include <catch_conversions/qt_catch_conversions.h>

#include <catch/catch.hpp>

#include <QChar>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;
using namespace QDOC_CATCH_GENERATORS_QCHAR_ALPHABETS_NAMESPACE;

SCENARIO("Binding a generated QChar to a range", "[QChar][Bounds]") {
    GIVEN("A lower bound") {
        auto lower_bound = GENERATE(take(100, random(
            static_cast<unsigned int>(std::numeric_limits<char16_t>::min()),
            static_cast<unsigned int>(std::numeric_limits<char16_t>::max())
        )));

        AND_GIVEN("An upper bound that is greater or equal than the lower bound") {
            auto upper_bound = GENERATE_COPY(take(100, random(lower_bound, static_cast<unsigned int>(std::numeric_limits<char16_t>::max()))));

            WHEN("A QChar is generated from those bounds") {
                QChar generated_character = GENERATE_COPY(take(1, character(lower_bound, upper_bound)));

                THEN("The generated character has a unicode value in the range [lower_bound, upper_bound]") {
                    REQUIRE(generated_character.unicode() >= lower_bound);
                    REQUIRE(generated_character.unicode() <= upper_bound);
                }
            }
        }
    }
}

TEST_CASE(
    "When lower_bound and upper_bound are equal, let their value be n, the only generated character is the one with unicode value n",
    "[QChar][Bounds]"
) {
    auto bound = GENERATE(take(100, random(
        static_cast<unsigned int>(std::numeric_limits<char16_t>::min()),
        static_cast<unsigned int>(std::numeric_limits<char16_t>::max())
    )));
    auto generated_character = GENERATE_COPY(take(100, character(bound, bound)));

    REQUIRE(generated_character.unicode() == bound);
}

TEST_CASE("When generating digits, each generated character is in the class [0-9]", "[QChar][SpecialCase]") {
    auto generated_character = GENERATE(take(100, digit()));

    REQUIRE(generated_character >= '0');
    REQUIRE(generated_character <= '9');
}

TEST_CASE("When generating lowercase ascii characters, each generated character is in the class [a-z]", "[QChar][SpecialCase]") {
    auto generated_character = GENERATE(take(100, ascii_lowercase()));

    REQUIRE(generated_character >= 'a');
    REQUIRE(generated_character <= 'z');
}

TEST_CASE("When generating uppercase ascii characters, each generated character is in the class [A-Z]", "[QChar][SpecialCase]") {
    auto generated_character = GENERATE(take(100, ascii_uppercase()));

    REQUIRE(generated_character >= 'A');
    REQUIRE(generated_character <= 'Z');
}

TEST_CASE("When generating ascii alphabetic characters, each generated character is in the class [a-zA-Z]", "[QChar][SpecialCase]") {
    auto generated_character = GENERATE(take(100, ascii_alpha()));

    REQUIRE((
             (generated_character >= 'a' && generated_character <= 'z') ||
             (generated_character >= 'A' && generated_character <= 'Z')
            ));
}

TEST_CASE("When generating ascii alphabetic characters, each generated character is in the class [a-zA-Z0-9]", "[QChar][SpecialCase]") {
    auto generated_character = GENERATE(take(100, ascii_alpha()));

    REQUIRE((
             (generated_character >= 'a' && generated_character <= 'z') ||
             (generated_character >= 'A' && generated_character <= 'Z') ||
             (generated_character >= '0' && generated_character <= '9')
            ));
}

TEST_CASE("When generating portable posix filename, each generated character is in the class [-_.a-zA-Z0-9]", "[QChar][SpecialCase]") {
    auto generated_character = GENERATE(take(100, ascii_alpha()));

    REQUIRE((
             (generated_character == '-') ||
             (generated_character == '_') ||
             (generated_character == '.') ||
             (generated_character >= 'a' && generated_character <= 'z') ||
             (generated_character >= 'A' && generated_character <= 'Z') ||
             (generated_character >= '0' && generated_character <= '9')
            ));
}
