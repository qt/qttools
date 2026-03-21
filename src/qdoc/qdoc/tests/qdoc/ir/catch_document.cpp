// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/document.h>
#include <qdoc/access.h>
#include <qdoc/genustypes.h>
#include <qdoc/status.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("IR::Document basic structure", "[IR::Document][IR]") {

    GIVEN("An empty IR::Document") {
        IR::Document ir;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The JSON contains all expected fields with empty or default values") {
                REQUIRE(json.contains("title"_L1));
                REQUIRE(json.contains("fullTitle"_L1));
                REQUIRE(json.contains("url"_L1));
                REQUIRE(!json.contains("brief"_L1));
                REQUIRE(json["title"_L1].toString().isEmpty());
                REQUIRE(json["fullTitle"_L1].toString().isEmpty());
                REQUIRE(json["url"_L1].toString().isEmpty());
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

            THEN("The content field contains only an empty blocks array when contentJson is empty") {
                REQUIRE(json.contains("content"_L1));
                QJsonObject content = json["content"_L1].toObject();
                REQUIRE(content.contains("blocks"_L1));
                REQUIRE(content["blocks"_L1].isArray());
                REQUIRE(content["blocks"_L1].toArray().isEmpty());
            }
        }
    }

    GIVEN("An IR::Document with basic metadata") {
        IR::Document ir;
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

SCENARIO("IR::Document classification metadata", "[IR::Document][IR][Classification]") {

    GIVEN("An IR::Document representing a C++ class") {
        IR::Document ir;
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

    GIVEN("An IR::Document representing a deprecated QML type") {
        IR::Document ir;
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

    GIVEN("An IR::Document representing a preliminary internal function") {
        IR::Document ir;
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

    GIVEN("An IR::Document representing an internal private property") {
        IR::Document ir;
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

    GIVEN("An IR::Document representing a DOC page") {
        IR::Document ir;
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

SCENARIO("IR::Document contentJson handling", "[IR::Document][IR][JSON]") {

    GIVEN("An IR::Document with empty contentJson") {
        IR::Document ir;
        ir.title = "Test"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The content field has an empty blocks array") {
                REQUIRE(json.contains("content"_L1));
                QJsonObject content = json["content"_L1].toObject();
                REQUIRE(content.contains("blocks"_L1));
                REQUIRE(content["blocks"_L1].toArray().isEmpty());
            }
        }
    }

    GIVEN("An IR::Document with simple contentJson") {
        IR::Document ir;
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
                // Always-emit: blocks array is also present alongside legacy keys
                REQUIRE(content.contains("blocks"_L1));
                REQUIRE(content["blocks"_L1].toArray().isEmpty());
            }

            THEN("The root JSON contains metadata alongside nested content") {
                REQUIRE(json.contains("title"_L1));
                REQUIRE(json.contains("content"_L1));
                REQUIRE(json["title"_L1].toString() == "Test");
            }
        }
    }

    GIVEN("An IR::Document with nested contentJson structure") {
        IR::Document ir;
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

SCENARIO("IR::Document since field serialization", "[IR::Document][IR][JSON]") {

    GIVEN("An IR::Document with no since value") {
        IR::Document ir;
        ir.title = "TestPage"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The since field is absent from JSON output") {
                REQUIRE(!json.contains("since"_L1));
            }
        }
    }

    GIVEN("An IR::Document with an empty since string") {
        IR::Document ir;
        ir.title = "TestPage"_L1;
        ir.since = ""_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The since field is absent from JSON output") {
                REQUIRE(!json.contains("since"_L1));
            }
        }
    }

    GIVEN("An IR::Document with a since version") {
        IR::Document ir;
        ir.title = "TestPage"_L1;
        ir.since = "6.8"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The since field is present with the correct value") {
                REQUIRE(json.contains("since"_L1));
                REQUIRE(json["since"_L1].toString() == "6.8");
            }
        }
    }

    GIVEN("An IR::Document with a since version containing a minor.patch version") {
        IR::Document ir;
        ir.title = "TestPage"_L1;
        ir.since = "6.8.2"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The full version string is preserved") {
                REQUIRE(json["since"_L1].toString() == "6.8.2");
            }
        }
    }
}

SCENARIO("IR::Document deprecatedSince field serialization", "[IR::Document][IR][JSON]") {

    GIVEN("An IR::Document with no deprecatedSince value") {
        IR::Document ir;
        ir.title = "TestPage"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The deprecatedSince field is absent from JSON output") {
                REQUIRE(!json.contains("deprecatedSince"_L1));
            }
        }
    }

    GIVEN("An IR::Document with an empty deprecatedSince string") {
        IR::Document ir;
        ir.title = "TestPage"_L1;
        ir.deprecatedSince = ""_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The deprecatedSince field is absent from JSON output") {
                REQUIRE(!json.contains("deprecatedSince"_L1));
            }
        }
    }

    GIVEN("An IR::Document with a deprecatedSince version") {
        IR::Document ir;
        ir.title = "TestPage"_L1;
        ir.deprecatedSince = "6.5"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The deprecatedSince field is present with the correct value") {
                REQUIRE(json.contains("deprecatedSince"_L1));
                REQUIRE(json["deprecatedSince"_L1].toString() == "6.5");
            }
        }
    }

    GIVEN("An IR::Document with a deprecatedSince version containing a minor.patch version") {
        IR::Document ir;
        ir.title = "TestPage"_L1;
        ir.deprecatedSince = "6.5.1"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The full version string is preserved") {
                REQUIRE(json["deprecatedSince"_L1].toString() == "6.5.1");
            }
        }
    }
}

SCENARIO("IR::Document complete workflow", "[IR::Document][IR][Integration]") {

    GIVEN("A fully populated IR::Document") {
        IR::Document ir;
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

SCENARIO("IR::Document body field with always-emit convention", "[IR::Document][IR][JSON]") {

    GIVEN("An IR::Document with no body content") {
        IR::Document ir;
        ir.title = "TestPage"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("The content field is present with an empty blocks array") {
                REQUIRE(json.contains("content"_L1));
                QJsonObject content = json["content"_L1].toObject();
                REQUIRE(content.contains("blocks"_L1));
                REQUIRE(content["blocks"_L1].isArray());
                REQUIRE(content["blocks"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("IR::Document hasQmlType flag", "[IR::Document][IR][QmlTypeInfo]") {

    GIVEN("An IR::Document without QML type info") {
        IR::Document ir;
        ir.title = "RegularPage"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("hasQmlType is false and qmlType key is absent") {
                REQUIRE(json["hasQmlType"_L1].toBool() == false);
                REQUIRE(!json.contains("qmlType"_L1));
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo minimal defaults", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo with only default values") {
        IR::QmlTypeInfo info;

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("Flags are false") {
                REQUIRE(json["isSingleton"_L1].toBool() == false);
                REQUIRE(json["isValueType"_L1].toBool() == false);
            }

            THEN("Optional fields are absent") {
                REQUIRE(!json.contains("importStatement"_L1));
                REQUIRE(!json.contains("inherits"_L1));
                REQUIRE(!json.contains("inheritedBy"_L1));
                REQUIRE(!json.contains("nativeType"_L1));
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo with import statement", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo with an import statement") {
        IR::QmlTypeInfo info;
        info.importStatement = "import QtQuick 2.15"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("The import statement is serialized") {
                REQUIRE(json["importStatement"_L1].toString() == "import QtQuick 2.15");
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo with inherits chain", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo with an inherited base type") {
        IR::QmlTypeInfo info;
        info.inherits = IR::QmlTypeInfo::InheritsInfo{
            "Item"_L1, "qml-qtquick-item.html"_L1, "QtQuick"_L1
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("The inherits object contains name, href, and moduleName") {
                REQUIRE(json.contains("inherits"_L1));
                QJsonObject inherits = json["inherits"_L1].toObject();
                REQUIRE(inherits["name"_L1].toString() == "Item");
                REQUIRE(inherits["href"_L1].toString() == "qml-qtquick-item.html");
                REQUIRE(inherits["moduleName"_L1].toString() == "QtQuick");
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo with inheritedBy list", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo with two subclass entries") {
        IR::QmlTypeInfo info;
        info.inheritedBy = {
            {"Rectangle"_L1, "qml-qtquick-rectangle.html"_L1},
            {"Text"_L1, "qml-qtquick-text.html"_L1}
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("The inheritedBy array has two entries with name and href") {
                REQUIRE(json.contains("inheritedBy"_L1));
                QJsonArray arr = json["inheritedBy"_L1].toArray();
                REQUIRE(arr.size() == 2);
                REQUIRE(arr[0].toObject()["name"_L1].toString() == "Rectangle");
                REQUIRE(arr[0].toObject()["href"_L1].toString() == "qml-qtquick-rectangle.html");
                REQUIRE(arr[1].toObject()["name"_L1].toString() == "Text");
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo with native type", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo with a native C++ type") {
        IR::QmlTypeInfo info;
        info.nativeType = IR::QmlTypeInfo::NativeTypeInfo{
            "QQuickItem"_L1, "qquickitem.html"_L1
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("The nativeType object contains name and href") {
                REQUIRE(json.contains("nativeType"_L1));
                QJsonObject nt = json["nativeType"_L1].toObject();
                REQUIRE(nt["name"_L1].toString() == "QQuickItem");
                REQUIRE(nt["href"_L1].toString() == "qquickitem.html");
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo with singleton flag", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo marked as singleton") {
        IR::QmlTypeInfo info;
        info.isSingleton = true;

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("isSingleton is true") {
                REQUIRE(json["isSingleton"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo with value type flag", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A QmlTypeInfo marked as value type") {
        IR::QmlTypeInfo info;
        info.isValueType = true;

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("isValueType is true") {
                REQUIRE(json["isValueType"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::QmlTypeInfo full population", "[IR::QmlTypeInfo][IR][QML]") {

    GIVEN("A fully populated QmlTypeInfo") {
        IR::QmlTypeInfo info;
        info.importStatement = "import QtQuick 2.15"_L1;
        info.isSingleton = false;
        info.isValueType = false;
        info.inherits = IR::QmlTypeInfo::InheritsInfo{
            "Item"_L1, "qml-qtquick-item.html"_L1, "QtQuick"_L1
        };
        info.inheritedBy = {
            {"Rectangle"_L1, "qml-qtquick-rectangle.html"_L1}
        };
        info.nativeType = IR::QmlTypeInfo::NativeTypeInfo{
            "QQuickItem"_L1, "qquickitem.html"_L1
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("All fields are present") {
                REQUIRE(json["importStatement"_L1].toString() == "import QtQuick 2.15");
                REQUIRE(json.contains("inherits"_L1));
                REQUIRE(json.contains("inheritedBy"_L1));
                REQUIRE(json.contains("nativeType"_L1));
            }
        }

        WHEN("Document includes QmlTypeInfo") {
            IR::Document ir;
            ir.title = "Item QML Type"_L1;
            ir.qmlTypeInfo = info;
            QJsonObject json = ir.toJson();

            THEN("hasQmlType is true and qmlType key is present") {
                REQUIRE(json["hasQmlType"_L1].toBool() == true);
                REQUIRE(json.contains("qmlType"_L1));
                REQUIRE(json["qmlType"_L1].toObject()["importStatement"_L1].toString()
                        == "import QtQuick 2.15");
            }
        }
    }
}

SCENARIO("IR::CollectionInfo with full C++ module metadata", "[IR::CollectionInfo][IR][JSON]") {

    GIVEN("A CollectionInfo with complete C++ module metadata") {
        IR::CollectionInfo info;
        info.logicalModuleName = "QtCore"_L1;
        info.logicalModuleVersion = "6.8"_L1;
        info.qtVariable = "core"_L1;
        info.cmakePackage = "Qt6"_L1;
        info.cmakeComponent = "Core"_L1;
        info.cmakeTargetItem = "Qt6::Core"_L1;
        info.state = "Technology Preview"_L1;
        info.isModule = true;
        info.isQmlModule = false;
        info.isGroup = false;
        info.noAutoList = false;

        info.namespaces = {
            {"QTest"_L1, "qtest.html"_L1, "Testing namespace"_L1},
            {"Qt"_L1, "qt-namespace.html"_L1, "Global Qt namespace"_L1}
        };

        info.classes = {
            {"QString"_L1, "qstring.html"_L1, "Unicode string class"_L1},
            {"QObject"_L1, "qobject.html"_L1, "Base class for Qt objects"_L1}
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("All module metadata fields are present") {
                REQUIRE(json["logicalModuleName"_L1].toString() == "QtCore");
                REQUIRE(json["logicalModuleVersion"_L1].toString() == "6.8");
                REQUIRE(json["state"_L1].toString() == "Technology Preview");
            }

            THEN("CMake/qmake variables are always emitted") {
                REQUIRE(json.contains("qtVariable"_L1));
                REQUIRE(json["qtVariable"_L1].toString() == "core");
                REQUIRE(json["cmakePackage"_L1].toString() == "Qt6");
                REQUIRE(json["cmakeComponent"_L1].toString() == "Core");
                REQUIRE(json["cmakeTargetItem"_L1].toString() == "Qt6::Core");
            }

            THEN("Type flags are always emitted as booleans") {
                REQUIRE(json.contains("isModule"_L1));
                REQUIRE(json.contains("isQmlModule"_L1));
                REQUIRE(json.contains("isGroup"_L1));
                REQUIRE(json["isModule"_L1].toBool() == true);
                REQUIRE(json["isQmlModule"_L1].toBool() == false);
                REQUIRE(json["isGroup"_L1].toBool() == false);
            }

            THEN("noAutoList flag is always emitted") {
                REQUIRE(json.contains("noAutoList"_L1));
                REQUIRE(json["noAutoList"_L1].toBool() == false);
            }

            THEN("Namespaces are serialized as an array of objects") {
                REQUIRE(json.contains("namespaces"_L1));
                QJsonArray arr = json["namespaces"_L1].toArray();
                REQUIRE(arr.size() == 2);
                REQUIRE(arr[0].toObject()["name"_L1].toString() == "QTest");
                REQUIRE(arr[0].toObject()["href"_L1].toString() == "qtest.html");
                REQUIRE(arr[0].toObject()["brief"_L1].toString() == "Testing namespace");
                REQUIRE(arr[1].toObject()["name"_L1].toString() == "Qt");
            }

            THEN("Classes are serialized as an array of objects") {
                REQUIRE(json.contains("classes"_L1));
                QJsonArray arr = json["classes"_L1].toArray();
                REQUIRE(arr.size() == 2);
                REQUIRE(arr[0].toObject()["name"_L1].toString() == "QString");
                REQUIRE(arr[0].toObject()["href"_L1].toString() == "qstring.html");
                REQUIRE(arr[0].toObject()["brief"_L1].toString() == "Unicode string class");
            }

            THEN("Members array is always emitted (empty for modules)") {
                REQUIRE(json.contains("members"_L1));
                REQUIRE(json["members"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("IR::CollectionInfo with minimal data (group)", "[IR::CollectionInfo][IR][JSON]") {

    GIVEN("A CollectionInfo for a group with no module metadata") {
        IR::CollectionInfo info;
        info.isModule = false;
        info.isQmlModule = false;
        info.isGroup = true;
        info.noAutoList = false;

        info.members = {
            {"QWidget"_L1, "qwidget.html"_L1, "Base class for widgets"_L1},
            {"QLabel"_L1, "qlabel.html"_L1, "Text or image display"_L1}
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("Type flags indicate a group") {
                REQUIRE(json["isModule"_L1].toBool() == false);
                REQUIRE(json["isQmlModule"_L1].toBool() == false);
                REQUIRE(json["isGroup"_L1].toBool() == true);
            }

            THEN("Conditional metadata fields are absent when empty") {
                REQUIRE(!json.contains("logicalModuleName"_L1));
                REQUIRE(!json.contains("logicalModuleVersion"_L1));
                REQUIRE(!json.contains("state"_L1));
            }

            THEN("CMake/qmake variables are always emitted (empty strings)") {
                REQUIRE(json.contains("qtVariable"_L1));
                REQUIRE(json["qtVariable"_L1].toString().isEmpty());
                REQUIRE(json.contains("cmakePackage"_L1));
                REQUIRE(json["cmakePackage"_L1].toString().isEmpty());
            }

            THEN("Members array is populated for group") {
                QJsonArray arr = json["members"_L1].toArray();
                REQUIRE(arr.size() == 2);
                REQUIRE(arr[0].toObject()["name"_L1].toString() == "QWidget");
                REQUIRE(arr[1].toObject()["name"_L1].toString() == "QLabel");
            }

            THEN("Namespaces and classes arrays are always emitted (empty)") {
                REQUIRE(json.contains("namespaces"_L1));
                REQUIRE(json["namespaces"_L1].toArray().isEmpty());
                REQUIRE(json.contains("classes"_L1));
                REQUIRE(json["classes"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("IR::CollectionInfo MemberEntry serialization", "[IR::CollectionInfo][IR][JSON]") {

    GIVEN("A CollectionInfo with a single member entry") {
        IR::CollectionInfo info;
        info.isGroup = true;
        info.members = {
            {"QTimer"_L1, "qtimer.html"_L1, "Repetitive and single-shot timers"_L1}
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("The member entry has name, href, and brief fields") {
                QJsonArray arr = json["members"_L1].toArray();
                REQUIRE(arr.size() == 1);
                QJsonObject entry = arr[0].toObject();
                REQUIRE(entry.contains("name"_L1));
                REQUIRE(entry.contains("href"_L1));
                REQUIRE(entry.contains("brief"_L1));
                REQUIRE(entry["name"_L1].toString() == "QTimer");
                REQUIRE(entry["href"_L1].toString() == "qtimer.html");
                REQUIRE(entry["brief"_L1].toString() == "Repetitive and single-shot timers");
            }
        }
    }
}

SCENARIO("IR::CollectionInfo noAutoList flag", "[IR::CollectionInfo][IR][JSON]") {

    GIVEN("A CollectionInfo with noAutoList set to true") {
        IR::CollectionInfo info;
        info.isModule = true;
        info.noAutoList = true;

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("The noAutoList flag is true") {
                REQUIRE(json["noAutoList"_L1].toBool() == true);
            }

            THEN("Member arrays are still present but empty") {
                REQUIRE(json.contains("namespaces"_L1));
                REQUIRE(json["namespaces"_L1].toArray().isEmpty());
                REQUIRE(json.contains("classes"_L1));
                REQUIRE(json["classes"_L1].toArray().isEmpty());
                REQUIRE(json.contains("members"_L1));
                REQUIRE(json["members"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("IR::CollectionInfo QML module type", "[IR::CollectionInfo][IR][JSON]") {

    GIVEN("A CollectionInfo for a QML module") {
        IR::CollectionInfo info;
        info.logicalModuleName = "QtQuick"_L1;
        info.logicalModuleVersion = "6.8"_L1;
        info.isQmlModule = true;

        info.members = {
            {"Item"_L1, "qml-qtquick-item.html"_L1, "Base visual type"_L1},
            {"Rectangle"_L1, "qml-qtquick-rectangle.html"_L1, "Rectangle with fill"_L1}
        };

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("isQmlModule is true and isModule is false") {
                REQUIRE(json["isQmlModule"_L1].toBool() == true);
                REQUIRE(json["isModule"_L1].toBool() == false);
            }

            THEN("Members are in the flat members array") {
                QJsonArray arr = json["members"_L1].toArray();
                REQUIRE(arr.size() == 2);
                REQUIRE(arr[0].toObject()["name"_L1].toString() == "Item");
            }

            THEN("Namespaces and classes are empty") {
                REQUIRE(json["namespaces"_L1].toArray().isEmpty());
                REQUIRE(json["classes"_L1].toArray().isEmpty());
            }
        }
    }
}

SCENARIO("IR::Document with collectionInfo", "[IR::Document][IR::CollectionInfo][IR][JSON]") {

    GIVEN("A Document with collectionInfo set") {
        IR::Document ir;
        ir.title = "Qt Core"_L1;
        ir.nodeType = NodeType::Module;
        ir.genus = Genus::CPP;

        IR::CollectionInfo collInfo;
        collInfo.logicalModuleName = "QtCore"_L1;
        collInfo.isModule = true;
        ir.collectionInfo = collInfo;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("hasCollection flag is true") {
                REQUIRE(json.contains("hasCollection"_L1));
                REQUIRE(json["hasCollection"_L1].toBool() == true);
            }

            THEN("The collection key contains the CollectionInfo JSON") {
                REQUIRE(json.contains("collection"_L1));
                REQUIRE(json["collection"_L1].isObject());
                QJsonObject collection = json["collection"_L1].toObject();
                REQUIRE(collection["logicalModuleName"_L1].toString() == "QtCore");
                REQUIRE(collection["isModule"_L1].toBool() == true);
            }
        }
    }

    GIVEN("A Document without collectionInfo") {
        IR::Document ir;
        ir.title = "MyClass"_L1;
        ir.nodeType = NodeType::Class;
        ir.genus = Genus::CPP;

        WHEN("Converting to JSON") {
            QJsonObject json = ir.toJson();

            THEN("hasCollection flag is false") {
                REQUIRE(json.contains("hasCollection"_L1));
                REQUIRE(json["hasCollection"_L1].toBool() == false);
            }

            THEN("The collection key is absent from JSON") {
                REQUIRE(!json.contains("collection"_L1));
            }
        }
    }
}

SCENARIO("IR::CollectionInfo with empty state", "[IR::CollectionInfo][IR][JSON]") {

    GIVEN("A CollectionInfo with no state set") {
        IR::CollectionInfo info;
        info.isModule = true;
        info.logicalModuleName = "QtWidgets"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = info.toJson();

            THEN("State field is absent from JSON") {
                REQUIRE(!json.contains("state"_L1));
            }

            THEN("Module name is present") {
                REQUIRE(json["logicalModuleName"_L1].toString() == "QtWidgets");
            }
        }
    }
}
