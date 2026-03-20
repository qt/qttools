// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/member.h>

#include <QJsonObject>
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
