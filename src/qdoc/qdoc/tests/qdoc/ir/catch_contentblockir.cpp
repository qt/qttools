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

SCENARIO("ContentBlockIR default construction", "[ContentBlockIR][IR]") {

    GIVEN("A default-constructed ContentBlockIR") {
        ContentBlockIR block;

        THEN("The type is Paragraph") {
            REQUIRE(block.type == BlockType::Paragraph);
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

SCENARIO("BlockType JSON serialization", "[ContentBlockIR][IR][JSON]") {

    GIVEN("A ContentBlockIR of each type") {
        auto checkTypeId = [](BlockType type, const QString &expectedId) {
            ContentBlockIR block;
            block.type = type;
            QJsonObject json = block.toJson();
            REQUIRE(json["type"_L1].toString() == expectedId);
        };

        THEN("All block types produce kebab-case IDs") {
            checkTypeId(BlockType::Paragraph, "paragraph"_L1);
            checkTypeId(BlockType::CodeBlock, "code-block"_L1);
            checkTypeId(BlockType::List, "list"_L1);
            checkTypeId(BlockType::ListItem, "list-item"_L1);
            checkTypeId(BlockType::Section, "section"_L1);
            checkTypeId(BlockType::SectionHeading, "section-heading"_L1);
            checkTypeId(BlockType::Note, "note"_L1);
            checkTypeId(BlockType::Warning, "warning"_L1);
            checkTypeId(BlockType::Important, "important"_L1);
            checkTypeId(BlockType::Details, "details"_L1);
            checkTypeId(BlockType::Brief, "brief"_L1);
            checkTypeId(BlockType::Div, "div"_L1);
            checkTypeId(BlockType::Quotation, "quotation"_L1);
            checkTypeId(BlockType::Legalese, "legalese"_L1);
            checkTypeId(BlockType::HorizontalRule, "horizontal-rule"_L1);
            checkTypeId(BlockType::Table, "table"_L1);
            checkTypeId(BlockType::TableRow, "table-row"_L1);
            checkTypeId(BlockType::TableCell, "table-cell"_L1);
            checkTypeId(BlockType::Raw, "raw"_L1);
        }
    }
}

SCENARIO("Paragraph with mixed inline content", "[ContentBlockIR][IR][JSON]") {

    GIVEN("A paragraph containing text, bold text, and inline code") {
        ContentBlockIR para;
        para.type = BlockType::Paragraph;

        InlineContentIR textBefore;
        textBefore.type = InlineType::Text;
        textBefore.text = "Hello "_L1;

        InlineContentIR boldChild;
        boldChild.type = InlineType::Text;
        boldChild.text = "world"_L1;

        InlineContentIR bold;
        bold.type = InlineType::Bold;
        bold.children = { boldChild };

        InlineContentIR textAfter;
        textAfter.type = InlineType::Text;
        textAfter.text = " with "_L1;

        InlineContentIR code;
        code.type = InlineType::Code;
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

            THEN("Bold element has children but no text in JSON") {
                QJsonArray inlines = json["inlines"_L1].toArray();
                QJsonObject boldJson = inlines[1].toObject();
                REQUIRE(boldJson.contains("children"_L1));
                REQUIRE(!boldJson.contains("text"_L1));
                QJsonArray boldChildren = boldJson["children"_L1].toArray();
                REQUIRE(boldChildren.size() == 1);
                REQUIRE(boldChildren[0].toObject()["text"_L1].toString() == "world");
            }

            THEN("Empty collections are omitted") {
                REQUIRE(!json.contains("attributes"_L1));
                REQUIRE(!json.contains("children"_L1));
            }
        }
    }
}

SCENARIO("Code block with language attribute", "[ContentBlockIR][IR][JSON]") {

    GIVEN("A code block with C++ content") {
        ContentBlockIR codeBlock;
        codeBlock.type = BlockType::CodeBlock;

        InlineContentIR codeText;
        codeText.type = InlineType::Text;
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

SCENARIO("Nested blocks: List with ListItems", "[ContentBlockIR][IR][JSON]") {

    GIVEN("A bullet list with two items") {
        ContentBlockIR list;
        list.type = BlockType::List;
        QJsonObject listAttrs;
        listAttrs["listType"_L1] = "bullet"_L1;
        list.attributes = listAttrs;

        // First item
        ContentBlockIR item1;
        item1.type = BlockType::ListItem;
        ContentBlockIR item1Para;
        item1Para.type = BlockType::Paragraph;
        InlineContentIR item1Text;
        item1Text.type = InlineType::Text;
        item1Text.text = "First item"_L1;
        item1Para.inlineContent = { item1Text };
        item1.children = { item1Para };

        // Second item
        ContentBlockIR item2;
        item2.type = BlockType::ListItem;
        ContentBlockIR item2Para;
        item2Para.type = BlockType::Paragraph;
        InlineContentIR item2Text;
        item2Text.type = InlineType::Text;
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

SCENARIO("Empty block produces minimal JSON", "[ContentBlockIR][IR][JSON]") {

    GIVEN("An empty paragraph with no content") {
        ContentBlockIR block;
        block.type = BlockType::Paragraph;

        WHEN("Converting to JSON") {
            QJsonObject json = block.toJson();

            THEN("Only type and text are present") {
                REQUIRE(json.contains("type"_L1));
                REQUIRE(json.contains("text"_L1));
                REQUIRE(json["text"_L1].toString().isEmpty());
            }

            THEN("Empty collections are omitted") {
                REQUIRE(!json.contains("attributes"_L1));
                REQUIRE(!json.contains("inlines"_L1));
                REQUIRE(!json.contains("children"_L1));
            }
        }
    }
}

SCENARIO("Leaf vs container invariant for block elements", "[ContentBlockIR][IR][Invariant]") {

    GIVEN("A leaf block (Paragraph) with inlineContent and no children") {
        ContentBlockIR leaf;
        leaf.type = BlockType::Paragraph;

        InlineContentIR text;
        text.type = InlineType::Text;
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

            THEN("children is absent") {
                REQUIRE(!json.contains("children"_L1));
            }
        }
    }

    GIVEN("A container block (Note) with children and no inlineContent") {
        ContentBlockIR note;
        note.type = BlockType::Note;

        ContentBlockIR para;
        para.type = BlockType::Paragraph;
        InlineContentIR paraText;
        paraText.type = InlineType::Text;
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
