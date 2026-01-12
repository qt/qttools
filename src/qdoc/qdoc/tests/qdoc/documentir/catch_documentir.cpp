// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/documentir.h>

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("DocumentIR basic structure", "[DocumentIR][IR]") {

    GIVEN("An empty DocumentIR") {
        DocumentIR ir;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The JSON contains all expected fields with empty values") {
                REQUIRE(json.contains("title"_L1));
                REQUIRE(json.contains("fullTitle"_L1));
                REQUIRE(json.contains("url"_L1));
                REQUIRE(json.contains("brief"_L1));
                REQUIRE(json["title"_L1].toString().isEmpty());
                REQUIRE(json["fullTitle"_L1].toString().isEmpty());
                REQUIRE(json["url"_L1].toString().isEmpty());
                REQUIRE(json["brief"_L1].toString().isEmpty());
            }

            THEN("The JSON does not contain a content field when contentJson is empty") {
                REQUIRE(!json.contains("content"_L1));
            }
        }
    }

    GIVEN("A DocumentIR with basic metadata") {
        DocumentIR ir;
        ir.title = "Test Page"_L1;
        ir.fullTitle = "Test Module::Test Page"_L1;
        ir.url = "test-page.html"_L1;
        ir.brief = "A brief description of the test page."_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("All metadata fields are correctly represented") {
                REQUIRE(json["title"_L1].toString() == "Test Page");
                REQUIRE(json["fullTitle"_L1].toString() == "Test Module::Test Page");
                REQUIRE(json["url"_L1].toString() == "test-page.html");
                REQUIRE(json["brief"_L1].toString() == "A brief description of the test page.");
            }
        }
    }
}

SCENARIO("DocumentIR contentJson handling", "[DocumentIR][IR][JSON]") {

    GIVEN("A DocumentIR with empty contentJson") {
        DocumentIR ir;
        ir.title = "Test"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The content field is not present") {
                REQUIRE(!json.contains("content"_L1));
            }
        }
    }

    GIVEN("A DocumentIR with simple contentJson") {
        DocumentIR ir;
        ir.title = "Test"_L1;
        ir.contentJson["text"_L1] = "Some content"_L1;
        ir.contentJson["count"_L1] = 42;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The content field is nested correctly") {
                REQUIRE(json.contains("content"_L1));
                REQUIRE(json["content"_L1].isObject());

                QJsonObject content = json["content"_L1].toObject();
                REQUIRE(content["text"_L1].toString() == "Some content");
                REQUIRE(content["count"_L1].toInt() == 42);
            }

            THEN("The root JSON contains metadata alongside nested content") {
                REQUIRE(json.contains("title"_L1));
                REQUIRE(json.contains("content"_L1));
                REQUIRE(json["title"_L1].toString() == "Test");
            }
        }
    }

    GIVEN("A DocumentIR with nested contentJson structure") {
        DocumentIR ir;
        ir.title = "Test"_L1;

        QJsonObject innerContent;
        innerContent["paragraph"_L1] = "First paragraph"_L1;
        innerContent["emphasis"_L1] = true;

        ir.contentJson["section"_L1] = innerContent;
        ir.contentJson["footer"_L1] = "Copyright 2025"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The nested structure is preserved under content key") {
                REQUIRE(json["content"_L1].isObject());

                QJsonObject content = json["content"_L1].toObject();
                REQUIRE(content.contains("section"_L1));
                REQUIRE(content["section"_L1].isObject());

                QJsonObject section = content["section"_L1].toObject();
                REQUIRE(section["paragraph"_L1].toString() == "First paragraph");
                REQUIRE(section["emphasis"_L1].toBool() == true);
                REQUIRE(content["footer"_L1].toString() == "Copyright 2025");
            }
        }
    }
}

SCENARIO("DocumentIR complete workflow", "[DocumentIR][IR][Integration]") {

    GIVEN("A fully populated DocumentIR") {
        DocumentIR ir;
        ir.title = "QTextStream"_L1;
        ir.fullTitle = "Qt Core::QTextStream"_L1;
        ir.url = "qtextstream.html"_L1;
        ir.brief = "The QTextStream class provides a convenient interface for reading and writing text."_L1;

        QJsonObject content;
        content["description"_L1] = "Detailed description here."_L1;
        content["memberCount"_L1] = 15;

        ir.contentJson["main"_L1] = content;
        ir.contentJson["related"_L1] = "QDataStream, QIODevice"_L1;

        WHEN("Converting to JSON for template rendering") {
            QJsonObject json = ir.toJson();

            THEN("All fields are present and properly structured") {
                // Check metadata fields at root level
                REQUIRE(json["title"_L1].toString() == "QTextStream");
                REQUIRE(json["fullTitle"_L1].toString() == "Qt Core::QTextStream");
                REQUIRE(json["url"_L1].toString() == "qtextstream.html");
                REQUIRE(json["brief"_L1].toString().startsWith("The QTextStream class"));

                // Check content is nested
                REQUIRE(json["content"_L1].isObject());
                QJsonObject contentObj = json["content"_L1].toObject();
                REQUIRE(contentObj.contains("main"_L1));
                REQUIRE(contentObj.contains("related"_L1));

                // Verify nested content structure
                QJsonObject mainContent = contentObj["main"_L1].toObject();
                REQUIRE(mainContent["description"_L1].toString() == "Detailed description here.");
                REQUIRE(mainContent["memberCount"_L1].toInt() == 15);

                REQUIRE(contentObj["related"_L1].toString() == "QDataStream, QIODevice");
            }

            THEN("The JSON structure follows template conventions") {
                // Metadata at root for easy access in templates: {{ title }}
                REQUIRE(json.contains("title"_L1));

                // Content nested for organization: {{ content.main.description }}
                REQUIRE(json["content"_L1].isObject());
            }
        }
    }
}
