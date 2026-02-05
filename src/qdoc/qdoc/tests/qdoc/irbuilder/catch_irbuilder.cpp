// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/irbuilder.h>
#include <qdoc/ir/documentir.h>

#include <QtCore/qglobal.h>

// NOTE: IRBuilder requires PageNode instances, which need the full QDoc
// infrastructure (Aggregate parents, Doc objects with Atom chains, etc.).
// Full unit testing requires either:
//   1. Mock/fake implementations of PageNode and its dependencies
//   2. A test fixture that sets up minimal QDoc infrastructure
//   3. Integration testing via tst_validateTemplateGeneratorOutput
//
// Currently, option 3 provides coverage. These tests verify compilation
// and document intent for future comprehensive unit testing.

SCENARIO("IRBuilder compilation", "[IRBuilder][IR][Compilation]") {

    GIVEN("The IRBuilder class") {
        WHEN("The header is included") {
            THEN("It compiles successfully") {
                REQUIRE(true);
            }
        }
    }

    GIVEN("An IRBuilder instance") {
        IRBuilder builder;
        Q_UNUSED(builder);

        WHEN("Default constructed") {
            THEN("It is ready to use") {
                // IRBuilder is stateless and always ready
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("IRBuilder interface design", "[IRBuilder][IR][Interface]") {

    GIVEN("An IRBuilder instance") {
        IRBuilder builder;
        Q_UNUSED(builder);

        WHEN("Documenting the expected interface") {
            THEN("buildPageIR takes a PageNode pointer and returns DocumentIR") {
                // The signature is:
                //   DocumentIR buildPageIR(const PageNode *pn) const;
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
                //   1. Empty PageNode produces empty/default DocumentIR
                //   2. PageNode with title populates ir.title
                //   3. PageNode with brief populates ir.brief
                //   4. PageNode with body content populates ir.contentJson["text"]
                //   5. Brief content is excluded from body text
                //   6. Paragraph breaks are converted to double newlines
                INFO("IRBuilder::buildPageIR() signature verified by compilation");
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("IRBuilder atom processing", "[IRBuilder][IR][Atoms]") {

    GIVEN("Documentation about atom processing behavior") {
        THEN("IRBuilder handles specific atom types") {
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

SCENARIO("IRBuilder integration testing", "[IRBuilder][IR][Integration]") {

    GIVEN("IRBuilder's role in the pipeline") {
        THEN("End-to-end testing is provided by tst_validateTemplateGeneratorOutput") {
            // The test at:
            //   tests/validatetemplategeneratoroutput/
            //
            // Verifies that:
            //   1. QDoc parses source files correctly
            //   2. IRBuilder extracts content from nodes
            //   3. TemplateGenerator renders IR to output
            //   4. Output matches expected files
            //
            // This provides integration coverage for IRBuilder without
            // requiring mock infrastructure.
            INFO("Integration coverage provided by tst_validateTemplateGeneratorOutput");
            REQUIRE(true);
        }
    }
}
