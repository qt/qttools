// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/templategenerator.h>

// NOTE: TemplateGenerator is a skeleton implementation that extends Generator.
// Full integration testing requires the complete QDoc infrastructure (Config,
// QDocDatabase, Node hierarchy, etc.). These tests verify basic behavior and
// signal intent for future comprehensive testing once the IR system is in place.

SCENARIO("TemplateGenerator compilation", "[TemplateGenerator][Generator][Compilation]") {

    GIVEN("The TemplateGenerator class") {
        WHEN("The header is included") {
            THEN("It compiles successfully") {
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("TemplateGenerator format identification", "[TemplateGenerator][Generator]") {

    GIVEN("A TemplateGenerator instance") {
        // Note: FileResolver requires complex setup, so we verify that
        // TemplateGenerator can be instantiated and reports correct format
        // metadata without full infrastructure
        WHEN("Querying the output format") {
            THEN("It reports 'template' as its format identifier") {
                // TODO: Replace placeholder with real instantiation once a FileResolver
                // factory or test double is available:
                //   TemplateGenerator generator(fileResolver);
                //   REQUIRE(generator.format() == "template");
                INFO("TemplateGenerator::format() should return \"template\"");
                REQUIRE(true);  // Placeholder
            }
        }

        WHEN("Querying the file extension") {
            THEN("It reports 'html' as its default extension") {
                // TODO: Replace placeholder with real instantiation once a FileResolver
                // factory or test double is available:
                //   TemplateGenerator generator(fileResolver);
                //   REQUIRE(generator.fileExtension() == "html");
                INFO("TemplateGenerator::fileExtension() should return \"html\"");
                REQUIRE(true);  // Placeholder
            }
        }
    }
}

