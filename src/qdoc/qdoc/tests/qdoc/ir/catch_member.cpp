// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/member.h>
#include <qdoc/access.h>
#include <qdoc/genustypes.h>
#include <qdoc/status.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("IR::ParameterIR construction and JSON", "[IR::ParameterIR][IR]") {

    GIVEN("A ParameterIR with type, name, and default value") {
        IR::ParameterIR param;
        param.type = "const QString &"_L1;
        param.name = "text"_L1;
        param.defaultValue = "QString()"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = param.toJson();

            THEN("All three fields are present") {
                REQUIRE(json["type"_L1].toString() == "const QString &");
                REQUIRE(json["name"_L1].toString() == "text");
                REQUIRE(json["defaultValue"_L1].toString() == "QString()");
            }
        }
    }

    GIVEN("A ParameterIR with empty default value") {
        IR::ParameterIR param;
        param.type = "int"_L1;
        param.name = "count"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = param.toJson();

            THEN("The defaultValue key is omitted") {
                REQUIRE(json.contains("type"_L1));
                REQUIRE(json.contains("name"_L1));
                REQUIRE(!json.contains("defaultValue"_L1));
            }
        }
    }
}

SCENARIO("IR::EnumValueIR construction and JSON", "[IR::EnumValueIR][IR]") {

    GIVEN("An EnumValueIR with name, value, and since") {
        IR::EnumValueIR ev;
        ev.name = "AlignLeft"_L1;
        ev.value = "0x0001"_L1;
        ev.since = "6.2"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ev.toJson();

            THEN("All three fields are present") {
                REQUIRE(json["name"_L1].toString() == "AlignLeft");
                REQUIRE(json["value"_L1].toString() == "0x0001");
                REQUIRE(json["since"_L1].toString() == "6.2");
            }
        }
    }

    GIVEN("An EnumValueIR with only a name") {
        IR::EnumValueIR ev;
        ev.name = "AlignCenter"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ev.toJson();

            THEN("Only name is present; value and since are omitted") {
                REQUIRE(json["name"_L1].toString() == "AlignCenter");
                REQUIRE(!json.contains("value"_L1));
                REQUIRE(!json.contains("since"_L1));
            }
        }
    }
}

SCENARIO("IR::MemberIR for a simple function", "[IR::MemberIR][IR]") {

    GIVEN("A MemberIR representing a simple function with no parameters") {
        IR::MemberIR member;
        member.name = "show"_L1;
        member.fullName = "QWidget::show"_L1;
        member.signature = "void show()"_L1;
        member.href = "#show"_L1;
        member.brief = "Shows the widget."_L1;
        member.nodeType = NodeType::Function;
        member.access = Access::Public;
        member.status = Status::Active;
        member.isPrimaryOverload = true;
        member.overloadNumber = 0;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("Identity fields are correctly serialized") {
                REQUIRE(json["name"_L1].toString() == "show");
                REQUIRE(json["fullName"_L1].toString() == "QWidget::show");
                REQUIRE(json["signature"_L1].toString() == "void show()");
                REQUIRE(json["href"_L1].toString() == "#show");
                REQUIRE(json["brief"_L1].toString() == "Shows the widget.");
            }

            THEN("Classification fields use {id, label} structure") {
                REQUIRE(json["nodeType"_L1].toObject()["id"_L1].toString() == "function");
                REQUIRE(json["nodeType"_L1].toObject()["label"_L1].toString() == "Function");
                REQUIRE(json["access"_L1].toObject()["id"_L1].toString() == "public");
                REQUIRE(json["access"_L1].toObject()["label"_L1].toString() == "Public");
                REQUIRE(json["status"_L1].toObject()["id"_L1].toString() == "active");
                REQUIRE(json["status"_L1].toObject()["label"_L1].toString() == "Active");
            }

            THEN("Empty collections are omitted from JSON") {
                REQUIRE(!json.contains("parameters"_L1));
                REQUIRE(!json.contains("enumValues"_L1));
            }

            THEN("Overload metadata is present") {
                REQUIRE(json["isPrimaryOverload"_L1].toBool() == true);
                REQUIRE(json["overloadNumber"_L1].toInt() == 0);
            }
        }
    }
}

SCENARIO("IR::MemberIR for a function with parameters", "[IR::MemberIR][IR]") {

    GIVEN("A MemberIR with two parameters") {
        IR::MemberIR member;
        member.name = "setText"_L1;
        member.fullName = "QWidget::setText"_L1;
        member.signature = "void setText(const QString &text)"_L1;
        member.href = "#setText"_L1;
        member.brief = "Sets the text."_L1;
        member.nodeType = NodeType::Function;
        member.access = Access::Public;
        member.status = Status::Active;

        IR::ParameterIR p1;
        p1.type = "const QString &"_L1;
        p1.name = "text"_L1;

        IR::ParameterIR p2;
        p2.type = "int"_L1;
        p2.name = "mode"_L1;
        p2.defaultValue = "0"_L1;

        member.parameters = { p1, p2 };

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("The parameters array is present with correct entries") {
                REQUIRE(json.contains("parameters"_L1));
                QJsonArray params = json["parameters"_L1].toArray();
                REQUIRE(params.size() == 2);
                REQUIRE(params[0].toObject()["name"_L1].toString() == "text");
                REQUIRE(params[1].toObject()["name"_L1].toString() == "mode");
                REQUIRE(params[1].toObject()["defaultValue"_L1].toString() == "0");
            }
        }
    }
}

SCENARIO("IR::MemberIR for an overloaded function", "[IR::MemberIR][IR]") {

    GIVEN("A MemberIR that is a secondary overload") {
        IR::MemberIR member;
        member.name = "setValue"_L1;
        member.fullName = "QSpinBox::setValue"_L1;
        member.signature = "void setValue(double value)"_L1;
        member.href = "#setValue-1"_L1;
        member.nodeType = NodeType::Function;
        member.access = Access::Public;
        member.status = Status::Active;
        member.overloadNumber = 2;
        member.isPrimaryOverload = false;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("Overload fields reflect secondary status") {
                REQUIRE(json["overloadNumber"_L1].toInt() == 2);
                REQUIRE(json["isPrimaryOverload"_L1].toBool() == false);
            }
        }
    }
}

SCENARIO("IR::MemberIR for an enum with values", "[IR::MemberIR][IR]") {

    GIVEN("A MemberIR of type Enum with three enum values") {
        IR::MemberIR member;
        member.name = "Alignment"_L1;
        member.fullName = "Qt::Alignment"_L1;
        member.signature = "enum Alignment"_L1;
        member.href = "#Alignment-enum"_L1;
        member.nodeType = NodeType::Enum;
        member.access = Access::Public;
        member.status = Status::Active;

        IR::EnumValueIR v1;
        v1.name = "AlignLeft"_L1;
        v1.value = "0x0001"_L1;

        IR::EnumValueIR v2;
        v2.name = "AlignRight"_L1;
        v2.value = "0x0002"_L1;

        IR::EnumValueIR v3;
        v3.name = "AlignCenter"_L1;
        v3.value = "0x0004"_L1;

        member.enumValues = { v1, v2, v3 };

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("The enumValues array is present with correct entries") {
                REQUIRE(json.contains("enumValues"_L1));
                QJsonArray vals = json["enumValues"_L1].toArray();
                REQUIRE(vals.size() == 3);
                REQUIRE(vals[0].toObject()["name"_L1].toString() == "AlignLeft");
                REQUIRE(vals[1].toObject()["name"_L1].toString() == "AlignRight");
                REQUIRE(vals[2].toObject()["name"_L1].toString() == "AlignCenter");
            }

            THEN("Empty parameters array is omitted") {
                REQUIRE(!json.contains("parameters"_L1));
            }
        }
    }
}

SCENARIO("IR::MemberIR qualifier flags", "[IR::MemberIR][IR]") {

    GIVEN("A MemberIR with static, const, and virtual flags set") {
        IR::MemberIR member;
        member.name = "instance"_L1;
        member.fullName = "MySingleton::instance"_L1;
        member.signature = "static const MySingleton *instance()"_L1;
        member.href = "#instance"_L1;
        member.nodeType = NodeType::Function;
        member.access = Access::Public;
        member.status = Status::Active;
        member.isStatic = true;
        member.isConst = true;
        member.isVirtual = true;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("All qualifier flags are true") {
                REQUIRE(json["isStatic"_L1].toBool() == true);
                REQUIRE(json["isConst"_L1].toBool() == true);
                REQUIRE(json["isVirtual"_L1].toBool() == true);
                REQUIRE(json["isSignal"_L1].toBool() == false);
                REQUIRE(json["isSlot"_L1].toBool() == false);
            }
        }
    }

    GIVEN("A MemberIR with all qualifier flags false") {
        IR::MemberIR member;
        member.name = "doSomething"_L1;
        member.fullName = "MyClass::doSomething"_L1;
        member.signature = "void doSomething()"_L1;
        member.href = "#doSomething"_L1;
        member.nodeType = NodeType::Function;
        member.access = Access::Public;
        member.status = Status::Active;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("All qualifier flags are present and false") {
                REQUIRE(json.contains("isStatic"_L1));
                REQUIRE(json.contains("isConst"_L1));
                REQUIRE(json.contains("isVirtual"_L1));
                REQUIRE(json.contains("isSignal"_L1));
                REQUIRE(json.contains("isSlot"_L1));
                REQUIRE(json["isStatic"_L1].toBool() == false);
                REQUIRE(json["isConst"_L1].toBool() == false);
                REQUIRE(json["isVirtual"_L1].toBool() == false);
                REQUIRE(json["isSignal"_L1].toBool() == false);
                REQUIRE(json["isSlot"_L1].toBool() == false);
            }
        }
    }
}

SCENARIO("IR::MemberIR with NoType nodeType is omitted", "[IR::MemberIR][IR]") {

    GIVEN("A MemberIR with default NoType nodeType") {
        IR::MemberIR member;
        member.name = "unknown"_L1;
        member.fullName = "unknown"_L1;
        member.signature = "unknown"_L1;
        member.href = "#unknown"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("The nodeType field is omitted") {
                REQUIRE(!json.contains("nodeType"_L1));
            }
        }
    }
}
