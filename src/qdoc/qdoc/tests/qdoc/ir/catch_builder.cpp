// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/builder.h>
#include <qdoc/ir/contentblock.h>
#include <qdoc/ir/document.h>
#include <qdoc/ir/pagemetadata.h>

#include <QtCore/qglobal.h>

using namespace Qt::Literals;

SCENARIO("Builder compiles and default-constructs", "[IR::Builder][IR]") {

    GIVEN("An IR::Builder instance") {
        IR::Builder builder;
        Q_UNUSED(builder);

        WHEN("Default constructed") {
            THEN("It is ready to use") {
                REQUIRE(true);
            }
        }
    }
}

SCENARIO("Builder populates Document from PageMetadata", "[IR::Builder][IR]") {

    GIVEN("A PageMetadata with populated fields") {
        IR::PageMetadata pm;
        pm.title = u"Test Page"_s;
        pm.fullTitle = u"Test Page Full Title"_s;
        pm.brief = u"A test page brief."_s;
        pm.since = u"6.8"_s;
        pm.deprecatedSince = u"6.5"_s;
        pm.url = u"test-page.html"_s;
        pm.nodeType = NodeType::Page;
        pm.genus = Genus::DontCare;
        pm.status = Status::Active;
        pm.access = Access::Public;

        WHEN("buildPageIR is called") {
            IR::Builder builder;
            IR::Document ir = builder.buildPageIR(pm);

            THEN("Document fields match PageMetadata") {
                REQUIRE(ir.title == pm.title);
                REQUIRE(ir.fullTitle == pm.fullTitle);
                REQUIRE(ir.brief == pm.brief);
                REQUIRE(ir.since == pm.since);
                REQUIRE(ir.deprecatedSince == pm.deprecatedSince);
                REQUIRE(ir.url == pm.url);
                REQUIRE(ir.nodeType == pm.nodeType);
                REQUIRE(ir.genus == pm.genus);
                REQUIRE(ir.status == pm.status);
                REQUIRE(ir.access == pm.access);
            }
        }
    }
}

SCENARIO("Builder copies body from PageMetadata", "[IR::Builder][IR]") {

    GIVEN("A PageMetadata with body content blocks") {
        IR::PageMetadata pm;
        pm.title = u"Body Test"_s;

        IR::ContentBlock para1;
        para1.type = IR::BlockType::Paragraph;
        para1.inlineContent.append({ IR::InlineType::Text, u"First paragraph."_s, {}, {}, {} });

        IR::ContentBlock para2;
        para2.type = IR::BlockType::Paragraph;
        para2.inlineContent.append({ IR::InlineType::Text, u"Second paragraph."_s, {}, {}, {} });

        pm.body.append(para1);
        pm.body.append(para2);

        WHEN("buildPageIR is called") {
            IR::Builder builder;
            IR::Document ir = builder.buildPageIR(pm);

            THEN("Document body matches PageMetadata body") {
                REQUIRE(ir.body.size() == 2);
                REQUIRE(ir.body[0].type == IR::BlockType::Paragraph);
                REQUIRE(ir.body[1].type == IR::BlockType::Paragraph);
                REQUIRE(ir.body[0].plainText() == u"First paragraph."_s);
                REQUIRE(ir.body[1].plainText() == u"Second paragraph."_s);
            }

            THEN("Flat text fallback joins paragraphs with double newlines") {
                REQUIRE(ir.contentJson["text"_L1].toString()
                        == u"First paragraph.\n\nSecond paragraph."_s);
            }
        }
    }
}

SCENARIO("Builder handles empty body", "[IR::Builder][IR]") {

    GIVEN("A PageMetadata with no body content") {
        IR::PageMetadata pm;
        pm.title = u"Empty Body"_s;

        WHEN("buildPageIR is called") {
            IR::Builder builder;
            IR::Document ir = builder.buildPageIR(pm);

            THEN("Document body is empty") {
                REQUIRE(ir.body.isEmpty());
            }

            THEN("Flat text fallback is empty") {
                REQUIRE(ir.contentJson["text"_L1].toString().isEmpty());
            }
        }
    }
}

SCENARIO("Builder has no Node-layer dependencies", "[IR::Builder][IR]") {

    GIVEN("The Builder interface") {
        THEN("buildPageIR accepts PageMetadata, not PageNode") {
            // Builder consumes a value-type struct (PageMetadata) instead
            // of Node subclass pointers. This enables unit testing with
            // plain struct construction and removes the IR layer's
            // dependency on the legacy Node hierarchy.
            IR::PageMetadata pm;
            IR::Builder builder;
            IR::Document ir = builder.buildPageIR(pm);
            REQUIRE(ir.title.isEmpty());
        }
    }
}
