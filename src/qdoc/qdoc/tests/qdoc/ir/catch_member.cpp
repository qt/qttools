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

SCENARIO("IR::SectionIR construction and JSON", "[IR::SectionIR][IR]") {

    GIVEN("A SectionIR with two members") {
        IR::MemberIR m1;
        m1.name = "show"_L1;
        m1.fullName = "QWidget::show"_L1;
        m1.signature = "void show()"_L1;
        m1.href = "#show"_L1;
        m1.nodeType = NodeType::Function;
        m1.access = Access::Public;
        m1.status = Status::Active;

        IR::MemberIR m2;
        m2.name = "hide"_L1;
        m2.fullName = "QWidget::hide"_L1;
        m2.signature = "void hide()"_L1;
        m2.href = "#hide"_L1;
        m2.nodeType = NodeType::Function;
        m2.access = Access::Public;
        m2.status = Status::Active;

        IR::SectionIR section;
        section.id = "public-functions"_L1;
        section.title = "Public Functions"_L1;
        section.singular = "public function"_L1;
        section.plural = "public functions"_L1;
        section.members = { m1, m2 };

        WHEN("Converting to JSON") {
            QJsonObject json = section.toJson();

            THEN("Section metadata is correctly serialized") {
                REQUIRE(json["id"_L1].toString() == "public-functions");
                REQUIRE(json["title"_L1].toString() == "Public Functions");
                REQUIRE(json["singular"_L1].toString() == "public function");
                REQUIRE(json["plural"_L1].toString() == "public functions");
            }

            THEN("Members array contains both entries") {
                REQUIRE(json.contains("members"_L1));
                QJsonArray members = json["members"_L1].toArray();
                REQUIRE(members.size() == 2);
                REQUIRE(members[0].toObject()["name"_L1].toString() == "show");
                REQUIRE(members[1].toObject()["name"_L1].toString() == "hide");
            }

            THEN("Empty reimplementedMembers and inheritedMembers are omitted") {
                REQUIRE(!json.contains("reimplementedMembers"_L1));
                REQUIRE(!json.contains("inheritedMembers"_L1));
            }
        }
    }
}

SCENARIO("IR::SectionIR with reimplemented members", "[IR::SectionIR][IR]") {

    GIVEN("A SectionIR with one member and one reimplemented member") {
        IR::MemberIR m1;
        m1.name = "paint"_L1;
        m1.fullName = "MyWidget::paint"_L1;
        m1.signature = "void paint(QPainter *painter)"_L1;
        m1.href = "#paint"_L1;
        m1.nodeType = NodeType::Function;
        m1.access = Access::Protected;
        m1.status = Status::Active;

        IR::MemberIR reimpl;
        reimpl.name = "sizeHint"_L1;
        reimpl.fullName = "MyWidget::sizeHint"_L1;
        reimpl.signature = "QSize sizeHint() const"_L1;
        reimpl.href = "#sizeHint"_L1;
        reimpl.nodeType = NodeType::Function;
        reimpl.access = Access::Public;
        reimpl.status = Status::Active;
        reimpl.isConst = true;

        IR::SectionIR section;
        section.id = "protected-functions"_L1;
        section.title = "Protected Functions"_L1;
        section.singular = "protected function"_L1;
        section.plural = "protected functions"_L1;
        section.members = { m1 };
        section.reimplementedMembers = { reimpl };

        WHEN("Converting to JSON") {
            QJsonObject json = section.toJson();

            THEN("Both members and reimplementedMembers arrays are present") {
                REQUIRE(json.contains("members"_L1));
                REQUIRE(json["members"_L1].toArray().size() == 1);
                REQUIRE(json.contains("reimplementedMembers"_L1));
                REQUIRE(json["reimplementedMembers"_L1].toArray().size() == 1);
                REQUIRE(json["reimplementedMembers"_L1].toArray()[0].toObject()["name"_L1].toString() == "sizeHint");
            }
        }
    }
}

SCENARIO("IR::SectionIR with inherited members", "[IR::SectionIR][IR]") {

    GIVEN("A SectionIR with an inherited members entry") {
        IR::InheritedMembersIR inherited;
        inherited.className = "QObject"_L1;
        inherited.count = 5;
        inherited.href = "qobject.html"_L1;

        IR::SectionIR section;
        section.id = "public-functions"_L1;
        section.title = "Public Functions"_L1;
        section.singular = "public function"_L1;
        section.plural = "public functions"_L1;
        section.inheritedMembers = { inherited };

        WHEN("Converting to JSON") {
            QJsonObject json = section.toJson();

            THEN("The inheritedMembers array is present") {
                REQUIRE(json.contains("inheritedMembers"_L1));
                QJsonArray arr = json["inheritedMembers"_L1].toArray();
                REQUIRE(arr.size() == 1);
                REQUIRE(arr[0].toObject()["className"_L1].toString() == "QObject");
                REQUIRE(arr[0].toObject()["count"_L1].toInt() == 5);
                REQUIRE(arr[0].toObject()["href"_L1].toString() == "qobject.html");
            }
        }
    }

    GIVEN("A SectionIR with no inherited members") {
        IR::SectionIR section;
        section.id = "signals"_L1;
        section.title = "Signals"_L1;
        section.singular = "signal"_L1;
        section.plural = "signals"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = section.toJson();

            THEN("The inheritedMembers key is omitted") {
                REQUIRE(!json.contains("inheritedMembers"_L1));
            }
        }
    }
}

SCENARIO("IR::MemberIR QML property with no flags", "[IR::MemberIR][IR][QML]") {

    GIVEN("A MemberIR representing a QML property with defaults") {
        IR::MemberIR member;
        member.name = "width"_L1;
        member.fullName = "Item::width"_L1;
        member.signature = "width : real"_L1;
        member.href = "#width-prop"_L1;
        member.nodeType = NodeType::QmlProperty;
        member.dataType = "real"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("dataType is present and boolean flags are absent") {
                REQUIRE(json["dataType"_L1].toString() == "real");
                REQUIRE(!json.contains("isAttached"_L1));
                REQUIRE(!json.contains("isDefault"_L1));
                REQUIRE(!json.contains("isReadOnly"_L1));
                REQUIRE(!json.contains("isRequired"_L1));
            }
        }
    }
}

SCENARIO("IR::MemberIR QML attached property", "[IR::MemberIR][IR][QML]") {

    GIVEN("A MemberIR representing an attached QML property") {
        IR::MemberIR member;
        member.name = "keys"_L1;
        member.fullName = "Keys::keys"_L1;
        member.signature = "keys : list<Key>"_L1;
        member.href = "#keys-attached-prop"_L1;
        member.nodeType = NodeType::QmlProperty;
        member.dataType = "list<Key>"_L1;
        member.isAttached = true;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("isAttached is true") {
                REQUIRE(json["isAttached"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::MemberIR QML default property", "[IR::MemberIR][IR][QML]") {

    GIVEN("A MemberIR representing a default QML property") {
        IR::MemberIR member;
        member.name = "data"_L1;
        member.fullName = "Item::data"_L1;
        member.signature = "data : list<Object>"_L1;
        member.href = "#data-prop"_L1;
        member.nodeType = NodeType::QmlProperty;
        member.dataType = "list<Object>"_L1;
        member.isDefault = true;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("isDefault is true") {
                REQUIRE(json["isDefault"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::MemberIR QML readonly property", "[IR::MemberIR][IR][QML]") {

    GIVEN("A MemberIR representing a read-only QML property") {
        IR::MemberIR member;
        member.name = "count"_L1;
        member.fullName = "ListView::count"_L1;
        member.signature = "count : int"_L1;
        member.href = "#count-prop"_L1;
        member.nodeType = NodeType::QmlProperty;
        member.dataType = "int"_L1;
        member.isReadOnly = true;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("isReadOnly is true") {
                REQUIRE(json["isReadOnly"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::MemberIR QML required property", "[IR::MemberIR][IR][QML]") {

    GIVEN("A MemberIR representing a required QML property") {
        IR::MemberIR member;
        member.name = "model"_L1;
        member.fullName = "Delegate::model"_L1;
        member.signature = "model : var"_L1;
        member.href = "#model-prop"_L1;
        member.nodeType = NodeType::QmlProperty;
        member.dataType = "var"_L1;
        member.isRequired = true;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("isRequired is true") {
                REQUIRE(json["isRequired"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::MemberIR QML property with all flags", "[IR::MemberIR][IR][QML]") {

    GIVEN("A MemberIR with all QML property flags set") {
        IR::MemberIR member;
        member.name = "special"_L1;
        member.fullName = "Test::special"_L1;
        member.signature = "special : string"_L1;
        member.href = "#special-prop"_L1;
        member.nodeType = NodeType::QmlProperty;
        member.dataType = "string"_L1;
        member.isAttached = true;
        member.isDefault = true;
        member.isReadOnly = true;
        member.isRequired = true;

        WHEN("Converting to JSON") {
            QJsonObject json = member.toJson();

            THEN("All four boolean flags are present and true") {
                REQUIRE(json["isAttached"_L1].toBool() == true);
                REQUIRE(json["isDefault"_L1].toBool() == true);
                REQUIRE(json["isReadOnly"_L1].toBool() == true);
                REQUIRE(json["isRequired"_L1].toBool() == true);
                REQUIRE(json["dataType"_L1].toString() == "string");
            }
        }
    }
}

SCENARIO("AllMemberEntry minimal serialization", "[AllMembersIR][IR]") {

    GIVEN("An AllMemberEntry with only signature and href") {
        IR::AllMemberEntry entry;
        entry.signature = "source : url"_L1;
        entry.href = "#source-prop"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = entry.toJson();

            THEN("signature and href are present, optional fields omitted") {
                REQUIRE(json["signature"_L1].toString() == "source : url");
                REQUIRE(json["href"_L1].toString() == "#source-prop");
                REQUIRE(!json.contains("hints"_L1));
                REQUIRE(!json.contains("isPropertyGroup"_L1));
                REQUIRE(!json.contains("children"_L1));
            }
        }
    }
}

SCENARIO("AllMemberEntry with hints", "[AllMembersIR][IR]") {

    GIVEN("An AllMemberEntry with read-only and default hints") {
        IR::AllMemberEntry entry;
        entry.signature = "count : int"_L1;
        entry.href = "#count-prop"_L1;
        entry.hints = { "read-only"_L1, "default"_L1 };

        WHEN("Converting to JSON") {
            QJsonObject json = entry.toJson();

            THEN("The hints array is present with both entries") {
                REQUIRE(json.contains("hints"_L1));
                QJsonArray hints = json["hints"_L1].toArray();
                REQUIRE(hints.size() == 2);
                REQUIRE(hints[0].toString() == "read-only");
                REQUIRE(hints[1].toString() == "default");
            }
        }
    }
}

SCENARIO("AllMemberEntry with property group and children", "[AllMembersIR][IR]") {

    GIVEN("An AllMemberEntry that is a property group with children") {
        IR::AllMemberEntry child1;
        child1.signature = "font.family : string"_L1;
        child1.href = "#font.family-prop"_L1;

        IR::AllMemberEntry child2;
        child2.signature = "font.pointSize : real"_L1;
        child2.href = "#font.pointSize-prop"_L1;

        IR::AllMemberEntry entry;
        entry.signature = "font group"_L1;
        entry.href = "#font-group"_L1;
        entry.isPropertyGroup = true;
        entry.children = { child1, child2 };

        WHEN("Converting to JSON") {
            QJsonObject json = entry.toJson();

            THEN("isPropertyGroup is true and children are serialized") {
                REQUIRE(json["isPropertyGroup"_L1].toBool() == true);
                REQUIRE(json.contains("children"_L1));
                QJsonArray children = json["children"_L1].toArray();
                REQUIRE(children.size() == 2);
                REQUIRE(children[0].toObject()["signature"_L1].toString() == "font.family : string");
                REQUIRE(children[1].toObject()["signature"_L1].toString() == "font.pointSize : real");
            }
        }
    }
}

SCENARIO("MemberGroup serialization", "[AllMembersIR][IR]") {

    GIVEN("A MemberGroup with a type name and entries") {
        IR::AllMemberEntry e1;
        e1.signature = "width : real"_L1;
        e1.href = "#width-prop"_L1;

        IR::MemberGroup group;
        group.typeName = "Item"_L1;
        group.typeHref = "qml-item.html"_L1;
        group.members = { e1 };

        WHEN("Converting to JSON") {
            QJsonObject json = group.toJson();

            THEN("typeName, typeHref, and members array are present") {
                REQUIRE(json["typeName"_L1].toString() == "Item");
                REQUIRE(json["typeHref"_L1].toString() == "qml-item.html");
                REQUIRE(json["members"_L1].toArray().size() == 1);
                REQUIRE(json["members"_L1].toArray()[0].toObject()["signature"_L1].toString() == "width : real");
            }
        }
    }
}

SCENARIO("AllMembersIR QML type with groups", "[AllMembersIR][IR]") {

    GIVEN("An AllMembersIR for a QML type with two member groups") {
        IR::AllMemberEntry ownEntry;
        ownEntry.signature = "source : url"_L1;
        ownEntry.href = "#source-prop"_L1;

        IR::MemberGroup ownGroup;
        ownGroup.members = { ownEntry };

        IR::AllMemberEntry inheritedEntry;
        inheritedEntry.signature = "width : real"_L1;
        inheritedEntry.href = "#width-prop"_L1;

        IR::MemberGroup inheritedGroup;
        inheritedGroup.typeName = "Item"_L1;
        inheritedGroup.typeHref = "qml-item.html"_L1;
        inheritedGroup.members = { inheritedEntry };

        IR::AllMembersIR allMembers;
        allMembers.typeName = "LottieAnimation"_L1;
        allMembers.typeHref = "qml-lottieanimation.html"_L1;
        allMembers.isQmlType = true;
        allMembers.memberGroups = { ownGroup, inheritedGroup };

        WHEN("Converting to JSON") {
            QJsonObject json = allMembers.toJson();

            THEN("isQmlType is true and memberGroups are serialized") {
                REQUIRE(json["typeName"_L1].toString() == "LottieAnimation");
                REQUIRE(json["typeHref"_L1].toString() == "qml-lottieanimation.html");
                REQUIRE(json["isQmlType"_L1].toBool() == true);
                REQUIRE(json.contains("memberGroups"_L1));
                REQUIRE(json["memberGroups"_L1].toArray().size() == 2);
                REQUIRE(!json.contains("members"_L1));
            }
        }
    }
}

SCENARIO("AllMembersIR C++ type with flat members", "[AllMembersIR][IR]") {

    GIVEN("An AllMembersIR for a C++ class with flat members") {
        IR::AllMemberEntry e1;
        e1.signature = "void show()"_L1;
        e1.href = "#show"_L1;

        IR::AllMemberEntry e2;
        e2.signature = "void hide()"_L1;
        e2.href = "#hide"_L1;

        IR::AllMembersIR allMembers;
        allMembers.typeName = "QWidget"_L1;
        allMembers.typeHref = "qwidget.html"_L1;
        allMembers.isQmlType = false;
        allMembers.members = { e1, e2 };

        WHEN("Converting to JSON") {
            QJsonObject json = allMembers.toJson();

            THEN("isQmlType is false and members are serialized") {
                REQUIRE(json["isQmlType"_L1].toBool() == false);
                REQUIRE(json.contains("members"_L1));
                REQUIRE(json["members"_L1].toArray().size() == 2);
                REQUIRE(!json.contains("memberGroups"_L1));
            }
        }
    }
}

SCENARIO("MemberGroup with empty typeName for own members", "[AllMembersIR][IR]") {

    GIVEN("A MemberGroup with empty typeName indicating own members") {
        IR::AllMemberEntry e1;
        e1.signature = "source : url"_L1;
        e1.href = "#source-prop"_L1;

        IR::MemberGroup group;
        group.members = { e1 };

        WHEN("Converting to JSON") {
            QJsonObject json = group.toJson();

            THEN("typeName is emitted as empty string") {
                REQUIRE(json["typeName"_L1].toString().isEmpty());
                REQUIRE(json["typeHref"_L1].toString().isEmpty());
                REQUIRE(json["members"_L1].toArray().size() == 1);
            }
        }
    }
}

SCENARIO("AllMembersIR empty members produce empty JSON", "[AllMembersIR][IR]") {

    GIVEN("An AllMembersIR with no members or groups") {
        IR::AllMembersIR allMembers;
        allMembers.typeName = "EmptyType"_L1;
        allMembers.typeHref = "empty.html"_L1;
        allMembers.isQmlType = true;

        WHEN("Converting to JSON") {
            QJsonObject json = allMembers.toJson();

            THEN("Both members and memberGroups are absent") {
                REQUIRE(json["typeName"_L1].toString() == "EmptyType");
                REQUIRE(json["isQmlType"_L1].toBool() == true);
                REQUIRE(!json.contains("members"_L1));
                REQUIRE(!json.contains("memberGroups"_L1));
            }
        }
    }
}
