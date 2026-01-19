// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/documentir.h>
#include <qdoc/access.h>
#include <qdoc/genustypes.h>
#include <qdoc/status.h>

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("DocumentIR basic structure", "[DocumentIR][IR]") {

    GIVEN("An empty DocumentIR") {
        DocumentIR ir;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The JSON contains all expected fields with empty or default values") {
                REQUIRE(json.contains("title"_L1));
                REQUIRE(json.contains("fullTitle"_L1));
                REQUIRE(json.contains("url"_L1));
                REQUIRE(json.contains("brief"_L1));
                REQUIRE(json["title"_L1].toString().isEmpty());
                REQUIRE(json["fullTitle"_L1].toString().isEmpty());
                REQUIRE(json["url"_L1].toString().isEmpty());
                REQUIRE(json["brief"_L1].toString().isEmpty());
            }

            THEN("The JSON contains classification fields with default values") {
                // nodeType and genus are omitted when unclassified (NoType/DontCare)
                REQUIRE(!json.contains("nodeType"_L1));
                REQUIRE(!json.contains("genus"_L1));
                // status and access are always present as {id, label} objects
                REQUIRE(json.contains("status"_L1));
                REQUIRE(json.contains("access"_L1));
                REQUIRE(json["status"_L1].toObject()["id"_L1].toString() == "active");
                REQUIRE(json["status"_L1].toObject()["label"_L1].toString() == "Active");
                REQUIRE(json["access"_L1].toObject()["id"_L1].toString() == "public");
                REQUIRE(json["access"_L1].toObject()["label"_L1].toString() == "Public");
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

SCENARIO("DocumentIR classification metadata", "[DocumentIR][IR][Classification]") {

    GIVEN("A DocumentIR representing a C++ class") {
        DocumentIR ir;
        ir.nodeType = NodeType::Class;
        ir.genus = Genus::CPP;
        ir.status = Status::Active;
        ir.access = Access::Public;
        ir.title = "MyClass"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("Classification fields are correctly serialized as id+label objects") {
                REQUIRE(json["nodeType"_L1].toObject()["id"_L1].toString() == "class");
                REQUIRE(json["nodeType"_L1].toObject()["label"_L1].toString() == "Class");
                REQUIRE(json["genus"_L1].toObject()["id"_L1].toString() == "cpp");
                REQUIRE(json["genus"_L1].toObject()["label"_L1].toString() == "C++");
                REQUIRE(json["status"_L1].toObject()["id"_L1].toString() == "active");
                REQUIRE(json["access"_L1].toObject()["id"_L1].toString() == "public");
            }
        }
    }

    GIVEN("A DocumentIR representing a deprecated QML type") {
        DocumentIR ir;
        ir.nodeType = NodeType::QmlType;
        ir.genus = Genus::QML;
        ir.status = Status::Deprecated;
        ir.access = Access::Public;
        ir.title = "LegacyItem"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("QML-specific classification is correctly serialized") {
                REQUIRE(json["nodeType"_L1].toObject()["id"_L1].toString() == "qml-type");
                REQUIRE(json["nodeType"_L1].toObject()["label"_L1].toString() == "QML type");
                REQUIRE(json["genus"_L1].toObject()["id"_L1].toString() == "qml");
                REQUIRE(json["status"_L1].toObject()["id"_L1].toString() == "deprecated");
            }
        }
    }

    GIVEN("A DocumentIR representing a preliminary internal function") {
        DocumentIR ir;
        ir.nodeType = NodeType::Function;
        ir.genus = Genus::CPP;
        ir.status = Status::Preliminary;
        ir.access = Access::Protected;
        ir.title = "experimentalMethod"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("Preliminary status and protected access are correctly serialized") {
                REQUIRE(json["nodeType"_L1].toObject()["id"_L1].toString() == "function");
                REQUIRE(json["status"_L1].toObject()["id"_L1].toString() == "preliminary");
                REQUIRE(json["access"_L1].toObject()["id"_L1].toString() == "protected");
            }
        }
    }

    GIVEN("A DocumentIR representing an internal private property") {
        DocumentIR ir;
        ir.nodeType = NodeType::Property;
        ir.genus = Genus::CPP;
        ir.status = Status::Internal;
        ir.access = Access::Private;
        ir.title = "m_internalData"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("Internal status and private access are correctly serialized") {
                REQUIRE(json["status"_L1].toObject()["id"_L1].toString() == "internal");
                REQUIRE(json["access"_L1].toObject()["id"_L1].toString() == "private");
            }
        }
    }

    GIVEN("A DocumentIR representing a DOC page") {
        DocumentIR ir;
        ir.nodeType = NodeType::Page;
        ir.genus = Genus::DOC;
        ir.status = Status::Active;
        ir.title = "Getting Started"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("DOC genus is correctly serialized") {
                REQUIRE(json["nodeType"_L1].toObject()["id"_L1].toString() == "page");
                REQUIRE(json["nodeType"_L1].toObject()["label"_L1].toString() == "Page");
                REQUIRE(json["genus"_L1].toObject()["id"_L1].toString() == "doc");
                REQUIRE(json["genus"_L1].toObject()["label"_L1].toString() == "Documentation");
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
