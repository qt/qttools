// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/signaturespan.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("SignatureSpan with Text role", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan with SpanRole::Text") {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Text;
        span.text = "const "_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = span.toJson();

            THEN("The role is 'text' and the text is present") {
                REQUIRE(json["role"_L1].toString() == "text");
                REQUIRE(json["text"_L1].toString() == "const ");
            }
        }
    }
}

SCENARIO("SignatureSpan with Type role and href", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan with SpanRole::Type and an href") {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Type;
        span.text = "QString"_L1;
        span.href = "qstring.html"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = span.toJson();

            THEN("The JSON contains role, text, and href") {
                REQUIRE(json["role"_L1].toString() == "type");
                REQUIRE(json["text"_L1].toString() == "QString");
                REQUIRE(json["href"_L1].toString() == "qstring.html");
            }
        }
    }
}

SCENARIO("SignatureSpan with empty href omits href from JSON", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan with no href set") {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Name;
        span.text = "setText"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = span.toJson();

            THEN("The href key is absent") {
                REQUIRE(!json.contains("href"_L1));
                REQUIRE(json.contains("role"_L1));
                REQUIRE(json.contains("text"_L1));
            }
        }
    }
}

SCENARIO("SignatureSpan with Extra role", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan with SpanRole::Extra") {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Extra;
        span.text = "[static]"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = span.toJson();

            THEN("The role is 'extra'") {
                REQUIRE(json["role"_L1].toString() == "extra");
                REQUIRE(json["text"_L1].toString() == "[static]");
            }
        }
    }
}

SCENARIO("SignatureSpan with nested children", "[IR::SignatureSpan][IR]") {

    GIVEN("A TemplateDecl span containing Type child spans") {
        IR::SignatureSpan typeParam;
        typeParam.role = IR::SpanRole::Type;
        typeParam.text = "T"_L1;

        IR::SignatureSpan separator;
        separator.role = IR::SpanRole::Text;
        separator.text = ", "_L1;

        IR::SignatureSpan typeParam2;
        typeParam2.role = IR::SpanRole::Type;
        typeParam2.text = "U"_L1;

        IR::SignatureSpan templateDecl;
        templateDecl.role = IR::SpanRole::TemplateDecl;
        templateDecl.text = "template"_L1;
        templateDecl.children = { typeParam, separator, typeParam2 };

        WHEN("Converting to JSON") {
            QJsonObject json = templateDecl.toJson();

            THEN("The children array is present with 3 elements") {
                REQUIRE(json["role"_L1].toString() == "template-decl");
                QJsonArray children = json["children"_L1].toArray();
                REQUIRE(children.size() == 3);
                REQUIRE(children[0].toObject()["role"_L1].toString() == "type");
                REQUIRE(children[0].toObject()["text"_L1].toString() == "T");
                REQUIRE(children[2].toObject()["role"_L1].toString() == "type");
                REQUIRE(children[2].toObject()["text"_L1].toString() == "U");
            }
        }
    }
}

SCENARIO("plainText concatenates flat span list", "[IR::SignatureSpan][IR]") {

    GIVEN("A list of spans representing 'void setText(text)'") {
        QList<IR::SignatureSpan> spans;

        auto addSpan = [&](IR::SpanRole role, const QString &text) {
            IR::SignatureSpan s;
            s.role = role;
            s.text = text;
            spans.append(s);
        };

        addSpan(IR::SpanRole::Type, "void"_L1);
        addSpan(IR::SpanRole::Text, " "_L1);
        addSpan(IR::SpanRole::Name, "setText"_L1);
        addSpan(IR::SpanRole::Text, "("_L1);
        addSpan(IR::SpanRole::Parameter, "text"_L1);
        addSpan(IR::SpanRole::Text, ")"_L1);

        WHEN("Concatenating plainText from all spans") {
            QString result;
            for (const auto &span : spans)
                result += span.plainText();

            THEN("The result is 'void setText(text)'") {
                REQUIRE(result == "void setText(text)");
            }
        }
    }
}

SCENARIO("plainText recurses into children", "[IR::SignatureSpan][IR]") {

    GIVEN("A TemplateDecl span matching builder output shape") {
        IR::SignatureSpan open;
        open.role = IR::SpanRole::Text;
        open.text = "<"_L1;

        IR::SignatureSpan kw1;
        kw1.role = IR::SpanRole::Text;
        kw1.text = "typename"_L1;

        IR::SignatureSpan space1;
        space1.role = IR::SpanRole::Text;
        space1.text = " "_L1;

        IR::SignatureSpan child1;
        child1.role = IR::SpanRole::Parameter;
        child1.text = "T"_L1;

        IR::SignatureSpan sep;
        sep.role = IR::SpanRole::Text;
        sep.text = ", "_L1;

        IR::SignatureSpan kw2;
        kw2.role = IR::SpanRole::Text;
        kw2.text = "typename"_L1;

        IR::SignatureSpan space2;
        space2.role = IR::SpanRole::Text;
        space2.text = " "_L1;

        IR::SignatureSpan child2;
        child2.role = IR::SpanRole::Parameter;
        child2.text = "U"_L1;

        IR::SignatureSpan close;
        close.role = IR::SpanRole::Text;
        close.text = ">"_L1;

        IR::SignatureSpan templateDecl;
        templateDecl.role = IR::SpanRole::TemplateDecl;
        templateDecl.text = "template"_L1;
        templateDecl.children = { open, kw1, space1, child1, sep, kw2, space2, child2, close };

        WHEN("Getting plainText") {
            QString result = templateDecl.plainText();

            THEN("It concatenates own text plus all children text") {
                REQUIRE(result == "template<typename T, typename U>");
            }
        }
    }
}

SCENARIO("Empty children list omitted from JSON", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan with no children") {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Text;
        span.text = "void"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = span.toJson();

            THEN("The children key is absent") {
                REQUIRE(!json.contains("children"_L1));
            }
        }
    }
}

SCENARIO("All SpanRole values produce correct kebab-case JSON role strings", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan for each role") {
        auto checkRole = [](IR::SpanRole role, const QString &expectedId) {
            IR::SignatureSpan span;
            span.role = role;
            QJsonObject json = span.toJson();
            REQUIRE(json["role"_L1].toString() == expectedId);
        };

        THEN("All roles map to correct kebab-case strings") {
            checkRole(IR::SpanRole::Text, "text"_L1);
            checkRole(IR::SpanRole::Type, "type"_L1);
            checkRole(IR::SpanRole::Name, "name"_L1);
            checkRole(IR::SpanRole::Parameter, "parameter"_L1);
            checkRole(IR::SpanRole::Operator, "operator"_L1);
            checkRole(IR::SpanRole::Extra, "extra"_L1);
            checkRole(IR::SpanRole::TemplateDecl, "template-decl"_L1);
            checkRole(IR::SpanRole::Link, "link"_L1);
            checkRole(IR::SpanRole::ExternalRef, "external-ref"_L1);
        }
    }
}

SCENARIO("SignatureSpan with Link role and href", "[IR::SignatureSpan][IR]") {

    GIVEN("A SignatureSpan with SpanRole::Link and an href") {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Link;
        span.text = "QWidget"_L1;
        span.href = "qwidget.html"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = span.toJson();

            THEN("The role is 'link' and href is present") {
                REQUIRE(json["role"_L1].toString() == "link");
                REQUIRE(json["text"_L1].toString() == "QWidget");
                REQUIRE(json["href"_L1].toString() == "qwidget.html");
            }
        }
    }
}
