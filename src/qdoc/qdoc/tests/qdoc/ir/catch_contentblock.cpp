// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/contentblock.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("IR::InlineContent default construction", "[IR::InlineContent][IR]") {

    GIVEN("A default-constructed IR::InlineContent") {
        IR::InlineContent inline_;

        THEN("The type is Text") {
            REQUIRE(inline_.type == IR::InlineType::Text);
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

SCENARIO("IR::InlineType JSON serialization", "[IR::InlineContent][IR][JSON]") {

    GIVEN("An IR::InlineContent of each type") {
        auto checkTypeId = [](IR::InlineType type, const QString &expectedId) {
            IR::InlineContent inline_;
            inline_.type = type;
            QJsonObject json = inline_.toJson();
            REQUIRE(json["type"_L1].toString() == expectedId);
        };

        THEN("All inline types produce kebab-case IDs") {
            checkTypeId(IR::InlineType::Text, "text"_L1);
            checkTypeId(IR::InlineType::Code, "code"_L1);
            checkTypeId(IR::InlineType::Link, "link"_L1);
            checkTypeId(IR::InlineType::Bold, "bold"_L1);
            checkTypeId(IR::InlineType::Italic, "italic"_L1);
            checkTypeId(IR::InlineType::Teletype, "teletype"_L1);
            checkTypeId(IR::InlineType::Underline, "underline"_L1);
            checkTypeId(IR::InlineType::Strikethrough, "strikethrough"_L1);
            checkTypeId(IR::InlineType::Subscript, "subscript"_L1);
            checkTypeId(IR::InlineType::Superscript, "superscript"_L1);
            checkTypeId(IR::InlineType::Parameter, "parameter"_L1);
            checkTypeId(IR::InlineType::LineBreak, "line-break"_L1);
            checkTypeId(IR::InlineType::Image, "image"_L1);
            checkTypeId(IR::InlineType::Keyword, "keyword"_L1);
            checkTypeId(IR::InlineType::Target, "target"_L1);
        }
    }
}

SCENARIO("Nested inline formatting", "[IR::InlineContent][IR][JSON]") {

    GIVEN("Bold containing italic containing text") {
        IR::InlineContent innerText;
        innerText.type = IR::InlineType::Text;
        innerText.text = "styled"_L1;

        IR::InlineContent italic;
        italic.type = IR::InlineType::Italic;
        italic.children = { innerText };

        IR::InlineContent bold;
        bold.type = IR::InlineType::Bold;
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

SCENARIO("IR::InlineContent link with optional fields", "[IR::InlineContent][IR][JSON]") {

    GIVEN("A link with href and title") {
        IR::InlineContent link;
        link.type = IR::InlineType::Link;
        link.href = "qstring.html"_L1;
        link.title = "QString documentation"_L1;

        IR::InlineContent linkText;
        linkText.type = IR::InlineType::Text;
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
        IR::InlineContent link;
        link.type = IR::InlineType::Link;
        link.href = "qstring.html"_L1;

        IR::InlineContent linkText;
        linkText.type = IR::InlineType::Text;
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

SCENARIO("LineBreak inline produces newline text", "[IR::InlineContent][IR]") {

    GIVEN("A LineBreak inline element") {
        IR::InlineContent lb;
        lb.type = IR::InlineType::LineBreak;

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

SCENARIO("Leaf vs container invariant for inline elements", "[IR::InlineContent][IR][Invariant]") {

    GIVEN("A leaf inline (Text) with text and no children") {
        IR::InlineContent leaf;
        leaf.type = IR::InlineType::Text;
        leaf.text = "hello"_L1;

        THEN("plainText returns the text") {
            REQUIRE(leaf.plainText() == "hello");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = leaf.toJson();

            THEN("text is present in JSON") {
                REQUIRE(json["text"_L1].toString() == "hello");
            }

            THEN("children is an empty array") {
                REQUIRE(json.contains("children"_L1));
                REQUIRE(json["children"_L1].toArray().isEmpty());
            }
        }
    }

    GIVEN("A container inline (Bold) with children and no text") {
        IR::InlineContent child;
        child.type = IR::InlineType::Text;
        child.text = "bold text"_L1;

        IR::InlineContent container;
        container.type = IR::InlineType::Bold;
        container.children = { child };

        THEN("text is empty on the container") {
            REQUIRE(container.text.isEmpty());
        }

        THEN("plainText concatenates children") {
            REQUIRE(container.plainText() == "bold text");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = container.toJson();

            THEN("text contains flattened child text") {
                REQUIRE(json.contains("text"_L1));
                REQUIRE(json["text"_L1].toString() == "bold text");
            }

            THEN("children is present") {
                REQUIRE(json.contains("children"_L1));
            }
        }
    }
}

SCENARIO("IR::ContentBlock default construction", "[IR::ContentBlock][IR]") {

    GIVEN("A default-constructed IR::ContentBlock") {
        IR::ContentBlock block;

        THEN("The type is Paragraph") {
            REQUIRE(block.type == IR::BlockType::Paragraph);
        }

        THEN("Collections are empty") {
            REQUIRE(block.attributes.isEmpty());
            REQUIRE(block.inlineContent.isEmpty());
            REQUIRE(block.children.isEmpty());
        }

        THEN("plainText returns empty string") {
            REQUIRE(block.plainText().isEmpty());
        }
    }
}

SCENARIO("IR::BlockType JSON serialization", "[IR::ContentBlock][IR][JSON]") {

    GIVEN("A IR::ContentBlock of each type") {
        auto checkTypeId = [](IR::BlockType type, const QString &expectedId) {
            IR::ContentBlock block;
            block.type = type;
            QJsonObject json = block.toJson();
            REQUIRE(json["type"_L1].toString() == expectedId);
        };

        THEN("All block types produce kebab-case IDs") {
            checkTypeId(IR::BlockType::Paragraph, "paragraph"_L1);
            checkTypeId(IR::BlockType::CodeBlock, "code-block"_L1);
            checkTypeId(IR::BlockType::List, "list"_L1);
            checkTypeId(IR::BlockType::ListItem, "list-item"_L1);
            checkTypeId(IR::BlockType::Section, "section"_L1);
            checkTypeId(IR::BlockType::SectionHeading, "section-heading"_L1);
            checkTypeId(IR::BlockType::Note, "note"_L1);
            checkTypeId(IR::BlockType::Warning, "warning"_L1);
            checkTypeId(IR::BlockType::Important, "important"_L1);
            checkTypeId(IR::BlockType::Details, "details"_L1);
            checkTypeId(IR::BlockType::Brief, "brief"_L1);
            checkTypeId(IR::BlockType::Div, "div"_L1);
            checkTypeId(IR::BlockType::Quotation, "quotation"_L1);
            checkTypeId(IR::BlockType::Legalese, "legalese"_L1);
            checkTypeId(IR::BlockType::HorizontalRule, "horizontal-rule"_L1);
            checkTypeId(IR::BlockType::Table, "table"_L1);
            checkTypeId(IR::BlockType::TableRow, "table-row"_L1);
            checkTypeId(IR::BlockType::TableCell, "table-cell"_L1);
            checkTypeId(IR::BlockType::Raw, "raw"_L1);
        }
    }
}

SCENARIO("Paragraph with mixed inline content", "[IR::ContentBlock][IR][JSON]") {

    GIVEN("A paragraph containing text, bold text, and inline code") {
        IR::ContentBlock para;
        para.type = IR::BlockType::Paragraph;

        IR::InlineContent textBefore;
        textBefore.type = IR::InlineType::Text;
        textBefore.text = "Hello "_L1;

        IR::InlineContent boldChild;
        boldChild.type = IR::InlineType::Text;
        boldChild.text = "world"_L1;

        IR::InlineContent bold;
        bold.type = IR::InlineType::Bold;
        bold.children = { boldChild };

        IR::InlineContent textAfter;
        textAfter.type = IR::InlineType::Text;
        textAfter.text = " with "_L1;

        IR::InlineContent code;
        code.type = IR::InlineType::Code;
        code.text = "QDoc"_L1;

        para.inlineContent = { textBefore, bold, textAfter, code };

        THEN("plainText concatenates all inline text") {
            REQUIRE(para.plainText() == "Hello world with QDoc");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = para.toJson();

            THEN("Type is paragraph") {
                REQUIRE(json["type"_L1].toString() == "paragraph");
            }

            THEN("Text is computed from inline content") {
                REQUIRE(json["text"_L1].toString() == "Hello world with QDoc");
            }

            THEN("Inlines array contains all inline elements") {
                QJsonArray inlines = json["inlines"_L1].toArray();
                REQUIRE(inlines.size() == 4);
                REQUIRE(inlines[0].toObject()["type"_L1].toString() == "text");
                REQUIRE(inlines[1].toObject()["type"_L1].toString() == "bold");
                REQUIRE(inlines[2].toObject()["type"_L1].toString() == "text");
                REQUIRE(inlines[3].toObject()["type"_L1].toString() == "code");
            }

            THEN("Bold element has children and flattened text in JSON") {
                QJsonArray inlines = json["inlines"_L1].toArray();
                QJsonObject boldJson = inlines[1].toObject();
                REQUIRE(boldJson.contains("children"_L1));
                REQUIRE(boldJson.contains("text"_L1));
                REQUIRE(boldJson["text"_L1].toString() == "world");
                QJsonArray boldChildren = boldJson["children"_L1].toArray();
                REQUIRE(boldChildren.size() == 1);
                REQUIRE(boldChildren[0].toObject()["text"_L1].toString() == "world");
            }

            THEN("Empty attributes are omitted, children always present") {
                REQUIRE(!json.contains("attributes"_L1));
                REQUIRE(json.contains("children"_L1));
                REQUIRE(json["children"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("Code block with language attribute", "[IR::ContentBlock][IR][JSON]") {

    GIVEN("A code block with C++ content") {
        IR::ContentBlock codeBlock;
        codeBlock.type = IR::BlockType::CodeBlock;

        IR::InlineContent codeText;
        codeText.type = IR::InlineType::Text;
        codeText.text = "int x = 42;"_L1;
        codeBlock.inlineContent = { codeText };

        QJsonObject attrs;
        attrs["language"_L1] = "cpp"_L1;
        codeBlock.attributes = attrs;

        THEN("plainText returns the code content") {
            REQUIRE(codeBlock.plainText() == "int x = 42;");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = codeBlock.toJson();

            THEN("Type is code-block") {
                REQUIRE(json["type"_L1].toString() == "code-block");
            }

            THEN("Attributes contain language") {
                REQUIRE(json["attributes"_L1].toObject()["language"_L1].toString() == "cpp");
            }

            THEN("Text contains the code content") {
                REQUIRE(json["text"_L1].toString() == "int x = 42;");
            }
        }
    }
}

SCENARIO("Nested blocks: List with ListItems", "[IR::ContentBlock][IR][JSON]") {

    GIVEN("A bullet list with two items") {
        IR::ContentBlock list;
        list.type = IR::BlockType::List;
        QJsonObject listAttrs;
        listAttrs["listType"_L1] = "bullet"_L1;
        list.attributes = listAttrs;

        // First item
        IR::ContentBlock item1;
        item1.type = IR::BlockType::ListItem;
        IR::ContentBlock item1Para;
        item1Para.type = IR::BlockType::Paragraph;
        IR::InlineContent item1Text;
        item1Text.type = IR::InlineType::Text;
        item1Text.text = "First item"_L1;
        item1Para.inlineContent = { item1Text };
        item1.children = { item1Para };

        // Second item
        IR::ContentBlock item2;
        item2.type = IR::BlockType::ListItem;
        IR::ContentBlock item2Para;
        item2Para.type = IR::BlockType::Paragraph;
        IR::InlineContent item2Text;
        item2Text.type = IR::InlineType::Text;
        item2Text.text = "Second item"_L1;
        item2Para.inlineContent = { item2Text };
        item2.children = { item2Para };

        list.children = { item1, item2 };

        THEN("plainText concatenates children with newlines") {
            REQUIRE(list.plainText() == "First item\nSecond item");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = list.toJson();

            THEN("Type is list") {
                REQUIRE(json["type"_L1].toString() == "list");
            }

            THEN("Attributes contain list type") {
                REQUIRE(json["attributes"_L1].toObject()["listType"_L1].toString() == "bullet");
            }

            THEN("Children array contains two list items") {
                QJsonArray children = json["children"_L1].toArray();
                REQUIRE(children.size() == 2);
                REQUIRE(children[0].toObject()["type"_L1].toString() == "list-item");
                REQUIRE(children[1].toObject()["type"_L1].toString() == "list-item");
            }

            THEN("List items have nested paragraph children") {
                QJsonArray children = json["children"_L1].toArray();
                QJsonObject firstItem = children[0].toObject();
                QJsonArray itemChildren = firstItem["children"_L1].toArray();
                REQUIRE(itemChildren.size() == 1);
                REQUIRE(itemChildren[0].toObject()["type"_L1].toString() == "paragraph");
            }

            THEN("Inlines are not present on container blocks") {
                REQUIRE(!json.contains("inlines"_L1));
            }
        }
    }
}

SCENARIO("Empty block produces minimal JSON", "[IR::ContentBlock][IR][JSON]") {

    GIVEN("An empty paragraph with no content") {
        IR::ContentBlock block;
        block.type = IR::BlockType::Paragraph;

        WHEN("Converting to JSON") {
            QJsonObject json = block.toJson();

            THEN("Only type and text are present") {
                REQUIRE(json.contains("type"_L1));
                REQUIRE(json.contains("text"_L1));
                REQUIRE(json["text"_L1].toString().isEmpty());
            }

            THEN("Empty attributes and inlines are omitted, children always present") {
                REQUIRE(!json.contains("attributes"_L1));
                REQUIRE(!json.contains("inlines"_L1));
                REQUIRE(json.contains("children"_L1));
                REQUIRE(json["children"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("Leaf vs container invariant for block elements", "[IR::ContentBlock][IR][Invariant]") {

    GIVEN("A leaf block (Paragraph) with inlineContent and no children") {
        IR::ContentBlock leaf;
        leaf.type = IR::BlockType::Paragraph;

        IR::InlineContent text;
        text.type = IR::InlineType::Text;
        text.text = "A paragraph."_L1;
        leaf.inlineContent = { text };

        THEN("plainText returns the inline text") {
            REQUIRE(leaf.plainText() == "A paragraph.");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = leaf.toJson();

            THEN("inlines is present") {
                REQUIRE(json.contains("inlines"_L1));
            }

            THEN("children is an empty array") {
                REQUIRE(json.contains("children"_L1));
                REQUIRE(json["children"_L1].toArray().isEmpty());
            }
        }
    }

    GIVEN("A container block (Note) with children and no inlineContent") {
        IR::ContentBlock note;
        note.type = IR::BlockType::Note;

        IR::ContentBlock para;
        para.type = IR::BlockType::Paragraph;
        IR::InlineContent paraText;
        paraText.type = IR::InlineType::Text;
        paraText.text = "Note content."_L1;
        para.inlineContent = { paraText };

        note.children = { para };

        THEN("inlineContent is empty on the container") {
            REQUIRE(note.inlineContent.isEmpty());
        }

        THEN("plainText concatenates children") {
            REQUIRE(note.plainText() == "Note content.");
        }

        WHEN("Converting to JSON") {
            QJsonObject json = note.toJson();

            THEN("inlines is absent from container JSON") {
                REQUIRE(!json.contains("inlines"_L1));
            }

            THEN("children is present") {
                REQUIRE(json.contains("children"_L1));
            }
        }
    }
}

SCENARIO("Table serialization annotates body rows with bodyIndex",
         "[IR::ContentBlock][IR][JSON]") {
    GIVEN("A Table with one header row followed by three body rows") {
        IR::ContentBlock table;
        table.type = IR::BlockType::Table;

        IR::ContentBlock header;
        header.type = IR::BlockType::TableHeaderRow;
        table.children.append(header);

        for (int i = 0; i < 3; ++i) {
            IR::ContentBlock row;
            row.type = IR::BlockType::TableRow;
            table.children.append(row);
        }

        WHEN("the Table is serialized to JSON") {
            QJsonObject json = table.toJson();

            THEN("rows is present and contains all four child rows") {
                REQUIRE(json.contains("rows"_L1));
                REQUIRE(json["rows"_L1].toArray().size() == 4);
            }

            THEN("the header row carries no bodyIndex attribute") {
                const auto headerJson = json["rows"_L1].toArray().at(0).toObject();
                REQUIRE(headerJson["type"_L1].toString() == u"table-header-row"_s);
                const auto attrs = headerJson.value("attributes"_L1).toObject();
                REQUIRE_FALSE(attrs.contains("bodyIndex"_L1));
            }

            THEN("body rows carry 1-based bodyIndex counted across body rows only") {
                const auto rows = json["rows"_L1].toArray();
                REQUIRE(rows.at(1).toObject()["attributes"_L1]
                                .toObject()["bodyIndex"_L1].toInt() == 1);
                REQUIRE(rows.at(2).toObject()["attributes"_L1]
                                .toObject()["bodyIndex"_L1].toInt() == 2);
                REQUIRE(rows.at(3).toObject()["attributes"_L1]
                                .toObject()["bodyIndex"_L1].toInt() == 3);
            }
        }
    }

    GIVEN("A Table with body rows only (no header)") {
        IR::ContentBlock table;
        table.type = IR::BlockType::Table;
        for (int i = 0; i < 2; ++i) {
            IR::ContentBlock row;
            row.type = IR::BlockType::TableRow;
            table.children.append(row);
        }

        WHEN("the Table is serialized to JSON") {
            const auto rows = table.toJson()["rows"_L1].toArray();

            THEN("body rows count from 1 (header-free tables behave the same)") {
                REQUIRE(rows.at(0).toObject()["attributes"_L1]
                                .toObject()["bodyIndex"_L1].toInt() == 1);
                REQUIRE(rows.at(1).toObject()["attributes"_L1]
                                .toObject()["bodyIndex"_L1].toInt() == 2);
            }
        }
    }

    GIVEN("A non-Table parent containing TableRow children") {
        IR::ContentBlock section;
        section.type = IR::BlockType::Section;
        IR::ContentBlock orphan;
        orphan.type = IR::BlockType::TableRow;
        section.children.append(orphan);

        WHEN("the parent is serialized to JSON") {
            const auto children = section.toJson()["children"_L1].toArray();

            THEN("the bodyIndex injection only fires under a Table parent") {
                const auto attrs = children.at(0).toObject()
                        .value("attributes"_L1).toObject();
                REQUIRE_FALSE(attrs.contains("bodyIndex"_L1));
            }
        }
    }
}

SCENARIO("LinkOrigin enum exposes three variants", "[IR::LinkOrigin][IR]") {

    GIVEN("The LinkOrigin enum") {
        THEN("Three distinct variants are exposed in stable order") {
            REQUIRE(static_cast<unsigned char>(IR::LinkOrigin::Auto) == 0);
            REQUIRE(static_cast<unsigned char>(IR::LinkOrigin::Explicit) == 1);
            REQUIRE(static_cast<unsigned char>(IR::LinkOrigin::Synthesized) == 2);
        }

        THEN("The underlying type is unsigned char") {
            REQUIRE(sizeof(IR::LinkOrigin) == sizeof(unsigned char));
        }
    }
}
