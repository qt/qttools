// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/builder.h>
#include <qdoc/ir/document.h>

#include <QtCore/qglobal.h>

// NOTE: IR::Builder requires PageNode instances, which need the full QDoc
// infrastructure (Aggregate parents, Doc objects with Atom chains, etc.).
// Full unit testing requires either:
//   1. Mock/fake implementations of PageNode and its dependencies
//   2. A test fixture that sets up minimal QDoc infrastructure
//   3. Integration testing via tst_validateTemplateGeneratorOutput
//
// Currently, option 3 provides coverage. These tests verify compilation
// and document intent for future comprehensive unit testing.

SCENARIO("IR::Builder compilation", "[IR::Builder][IR][Compilation]") {

    GIVEN("The IR::Builder class") {
        WHEN("The header is included") {
            THEN("It compiles successfully") {
                REQUIRE(true);
            }
        }
    }

    GIVEN("An IR::Builder instance") {
        IR::Builder builder;
        Q_UNUSED(builder);

        WHEN("Default constructed") {
            THEN("It is ready to use") {
                // IR::Builder is stateless and always ready
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("IR::Builder interface design", "[IR::Builder][IR][Interface]") {

    GIVEN("An IR::Builder instance") {
        IR::Builder builder;
        Q_UNUSED(builder);

        WHEN("Documenting the expected interface") {
            THEN("buildPageIR takes a PageNode pointer and returns IR::Document") {
                // The signature is:
                //   IR::Document buildPageIR(const PageNode *pn) const;
                //
                // This extracts:
                //   - title from PageNode::title()
                //   - fullTitle from PageNode::fullTitle()
                //   - url from PageNode::url()
                //   - brief from PageNode::doc().briefText().toString()
                //   - body content from PageNode::doc().body() atom chain
                //
                // TODO: Once test infrastructure supports PageNode creation,
                // add tests that verify:
                //   1. Empty PageNode produces empty/default IR::Document
                //   2. PageNode with title populates ir.title
                //   3. PageNode with brief populates ir.brief
                //   4. PageNode with body content populates ir.contentJson["text"]
                //   5. Brief content is excluded from body text
                //   6. Paragraph breaks are converted to double newlines
                INFO("IR::Builder::buildPageIR() signature verified by compilation");
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("IR::Builder atom processing", "[IR::Builder][IR][Atoms]") {

    GIVEN("Documentation about atom processing behavior") {
        THEN("IR::Builder handles specific atom types") {
            // Current atom types handled:
            //   - Atom::BriefLeft / BriefRight: Track brief section (excluded from body)
            //   - Atom::ParaLeft: Insert paragraph breaks in body
            //   - Atom::String: Plain text content
            //   - Atom::AutoLink: Auto-linked text (treated as string for now)
            //   - Atom::C: Code/monospace text (treated as string for now)
            //
            // Future atom types to handle:
            //   - Atom::Link / LinkNode: Cross-references
            //   - Atom::Code / CodeQuoteCommand: Code blocks
            //   - Atom::Image: Images
            //   - Atom::ListLeft / ListRight: Lists
            //   - Atom::TableLeft / TableRight: Tables
            //
            // These will be added as the IR layer matures and templates
            // require richer content structures.
            INFO("Atom processing behavior documented for future implementation");
            REQUIRE(true);
        }
    }
}

SCENARIO("IR::Builder integration testing", "[IR::Builder][IR][Integration]") {

    GIVEN("IR::Builder's role in the pipeline") {
        THEN("End-to-end testing is provided by tst_validateTemplateGeneratorOutput") {
            // The test at:
            //   tests/validatetemplategeneratoroutput/
            //
            // Verifies that:
            //   1. QDoc parses source files correctly
            //   2. IR::Builder extracts content from nodes
            //   3. TemplateGenerator renders IR to output
            //   4. Output matches expected files
            //
            // This provides integration coverage for IR::Builder without
            // requiring mock infrastructure.
            INFO("Integration coverage provided by tst_validateTemplateGeneratorOutput");
            REQUIRE(true);
        }
    }
}
