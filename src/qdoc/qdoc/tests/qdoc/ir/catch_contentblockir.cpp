// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/contentblockir.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("InlineContentIR default construction", "[InlineContentIR][IR]") {

    GIVEN("A default-constructed InlineContentIR") {
        InlineContentIR inline_;

        THEN("The type is Text") {
            REQUIRE(inline_.type == InlineType::Text);
        }

        THEN("Text fields are empty") {
            REQUIRE(inline_.text.isEmpty());
            REQUIRE(inline_.href.isEmpty());
            REQUIRE(inline_.title.isEmpty());
        }

        THEN("Children are empty") {
            REQUIRE(inline_.children.isEmpty());
        }

        THEN("plainText returns empty string") {
            REQUIRE(inline_.plainText().isEmpty());
        }
    }
}

SCENARIO("InlineType JSON serialization", "[InlineContentIR][IR][JSON]") {

    GIVEN("An InlineContentIR of each type") {
        auto checkTypeId = [](InlineType type, const QString &expectedId) {
            InlineContentIR inline_;
            inline_.type = type;
            QJsonObject json = inline_.toJson();
            REQUIRE(json["type"_L1].toString() == expectedId);
        };

        THEN("All inline types produce kebab-case IDs") {
            checkTypeId(InlineType::Text, "text"_L1);
            checkTypeId(InlineType::Code, "code"_L1);
            checkTypeId(InlineType::Link, "link"_L1);
            checkTypeId(InlineType::Bold, "bold"_L1);
            checkTypeId(InlineType::Italic, "italic"_L1);
            checkTypeId(InlineType::Teletype, "teletype"_L1);
            checkTypeId(InlineType::Underline, "underline"_L1);
            checkTypeId(InlineType::Strikethrough, "strikethrough"_L1);
            checkTypeId(InlineType::Subscript, "subscript"_L1);
            checkTypeId(InlineType::Superscript, "superscript"_L1);
            checkTypeId(InlineType::Parameter, "parameter"_L1);
            checkTypeId(InlineType::LineBreak, "line-break"_L1);
            checkTypeId(InlineType::Image, "image"_L1);
            checkTypeId(InlineType::Keyword, "keyword"_L1);
            checkTypeId(InlineType::Target, "target"_L1);
        }
    }
}

SCENARIO("Nested inline formatting", "[InlineContentIR][IR][JSON]") {

    GIVEN("Bold containing italic containing text") {
        InlineContentIR innerText;
        innerText.type = InlineType::Text;
        innerText.text = "styled"_L1;

        InlineContentIR italic;
        italic.type = InlineType::Italic;
        italic.children = { innerText };

        InlineContentIR bold;
        bold.type = InlineType::Bold;
        bold.children = { italic };

        THEN("plainText traverses the full nesting") {
            REQUIRE(bold.plainText() == "styled");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = bold.toJson();

            THEN("Bold has children") {
                REQUIRE(json.contains("children"_L1));
                QJsonArray children = json["children"_L1].toArray();
                REQUIRE(children.size() == 1);
            }

            THEN("Italic is nested inside bold") {
                QJsonArray boldChildren = json["children"_L1].toArray();
                QJsonObject italicJson = boldChildren[0].toObject();
                REQUIRE(italicJson["type"_L1].toString() == "italic");
                REQUIRE(italicJson.contains("children"_L1));
            }

            THEN("Text is at the innermost level") {
                QJsonArray boldChildren = json["children"_L1].toArray();
                QJsonObject italicJson = boldChildren[0].toObject();
                QJsonArray italicChildren = italicJson["children"_L1].toArray();
                QJsonObject textJson = italicChildren[0].toObject();
                REQUIRE(textJson["type"_L1].toString() == "text");
                REQUIRE(textJson["text"_L1].toString() == "styled");
            }
        }
    }
}

SCENARIO("InlineContentIR link with optional fields", "[InlineContentIR][IR][JSON]") {

    GIVEN("A link with href and title") {
        InlineContentIR link;
        link.type = InlineType::Link;
        link.href = "qstring.html"_L1;
        link.title = "QString documentation"_L1;

        InlineContentIR linkText;
        linkText.type = InlineType::Text;
        linkText.text = "QString"_L1;
        link.children = { linkText };

        THEN("plainText returns the link text") {
            REQUIRE(link.plainText() == "QString");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("href and title are present") {
                REQUIRE(json["href"_L1].toString() == "qstring.html");
                REQUIRE(json["title"_L1].toString() == "QString documentation");
            }
        }
    }

    GIVEN("A link without optional title") {
        InlineContentIR link;
        link.type = InlineType::Link;
        link.href = "qstring.html"_L1;

        InlineContentIR linkText;
        linkText.type = InlineType::Text;
        linkText.text = "QString"_L1;
        link.children = { linkText };

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("title is omitted from JSON") {
                REQUIRE(!json.contains("title"_L1));
            }

            THEN("href is present") {
                REQUIRE(json["href"_L1].toString() == "qstring.html");
            }
        }
    }
}

SCENARIO("LineBreak inline produces newline text", "[InlineContentIR][IR]") {

    GIVEN("A LineBreak inline element") {
        InlineContentIR lb;
        lb.type = InlineType::LineBreak;

        THEN("plainText returns a newline") {
            REQUIRE(lb.plainText() == "\n");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = lb.toJson();

            THEN("text contains a newline") {
                REQUIRE(json["text"_L1].toString() == "\n");
            }
        }
    }
}

SCENARIO("Leaf vs container invariant for inline elements", "[InlineContentIR][IR][Invariant]") {

    GIVEN("A leaf inline (Text) with text and no children") {
        InlineContentIR leaf;
        leaf.type = InlineType::Text;
        leaf.text = "hello"_L1;

        THEN("plainText returns the text") {
            REQUIRE(leaf.plainText() == "hello");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = leaf.toJson();

            THEN("text is present in JSON") {
                REQUIRE(json["text"_L1].toString() == "hello");
            }

            THEN("children is absent") {
                REQUIRE(!json.contains("children"_L1));
            }
        }
    }

    GIVEN("A container inline (Bold) with children and no text") {
        InlineContentIR child;
        child.type = InlineType::Text;
        child.text = "bold text"_L1;

        InlineContentIR container;
        container.type = InlineType::Bold;
        container.children = { child };

        THEN("text is empty on the container") {
            REQUIRE(container.text.isEmpty());
        }

        THEN("plainText concatenates children") {
            REQUIRE(container.plainText() == "bold text");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = container.toJson();

            THEN("text is absent from container JSON") {
                REQUIRE(!json.contains("text"_L1));
            }

            THEN("children is present") {
                REQUIRE(json.contains("children"_L1));
            }
        }
    }
}
