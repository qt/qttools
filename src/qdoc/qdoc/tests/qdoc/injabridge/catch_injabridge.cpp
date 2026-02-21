// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/injabridge.h>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>
#include <QTemporaryFile>

using namespace Qt::Literals::StringLiterals;

SCENARIO("Converting QJsonValue to nlohmann::json", "[InjaBridge][JSON]") {

    GIVEN("A null QJsonValue") {
        QJsonValue value{QJsonValue::Null};

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(value);

            THEN("The result is a null json value") {
                REQUIRE(result.is_null());
            }
        }
    }

    GIVEN("A boolean QJsonValue") {
        QJsonValue value_true{true};
        QJsonValue value_false{false};

        WHEN("Converting to nlohmann::json") {
            auto result_true = InjaBridge::toInjaJson(value_true);
            auto result_false = InjaBridge::toInjaJson(value_false);

            THEN("The results are boolean json values with correct values") {
                REQUIRE(result_true.is_boolean());
                REQUIRE(result_true.get<bool>() == true);
                REQUIRE(result_false.is_boolean());
                REQUIRE(result_false.get<bool>() == false);
            }
        }
    }

    GIVEN("A numeric QJsonValue") {
        QJsonValue value{42.5};

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(value);

            THEN("The result is a numeric json value") {
                REQUIRE(result.is_number());
                REQUIRE(result.get<double>() == 42.5);
            }
        }
    }

    GIVEN("A string QJsonValue") {
        QJsonValue value{"Hello, World!"_L1};

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(value);

            THEN("The result is a string json value") {
                REQUIRE(result.is_string());
                REQUIRE(result.get<std::string>() == "Hello, World!");
            }
        }
    }

    GIVEN("An undefined QJsonValue") {
        QJsonValue value{QJsonValue::Undefined};

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(value);

            THEN("The result is a null json value") {
                REQUIRE(result.is_null());
            }
        }
    }
}

SCENARIO("Converting QJsonArray to nlohmann::json", "[InjaBridge][JSON]") {

    GIVEN("An empty QJsonArray") {
        QJsonArray array{};

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(array);

            THEN("The result is an empty json array") {
                REQUIRE(result.is_array());
                REQUIRE(result.empty());
            }
        }
    }

    GIVEN("A QJsonArray with mixed types") {
        QJsonArray array{};
        array.append(42);
        array.append("test"_L1);
        array.append(true);

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(array);

            THEN("The result is a json array with correct values") {
                REQUIRE(result.is_array());
                REQUIRE(result.size() == 3);
                REQUIRE(result[0].get<int>() == 42);
                REQUIRE(result[1].get<std::string>() == "test");
                REQUIRE(result[2].get<bool>() == true);
            }
        }
    }
}

SCENARIO("Converting QJsonObject to nlohmann::json", "[InjaBridge][JSON]") {

    GIVEN("An empty QJsonObject") {
        QJsonObject obj{};

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(obj);

            THEN("The result is an empty json object") {
                REQUIRE(result.is_object());
                REQUIRE(result.empty());
            }
        }
    }

    GIVEN("A QJsonObject with mixed types") {
        QJsonObject obj{};
        obj["number"_L1] = 42;
        obj["text"_L1] = "hello"_L1;
        obj["flag"_L1] = true;

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(obj);

            THEN("The result is a json object with correct key-value pairs") {
                REQUIRE(result.is_object());
                REQUIRE(result.size() == 3);
                REQUIRE(result["number"].get<int>() == 42);
                REQUIRE(result["text"].get<std::string>() == "hello");
                REQUIRE(result["flag"].get<bool>() == true);
            }
        }
    }

    GIVEN("A nested QJsonObject") {
        QJsonObject inner{};
        inner["value"_L1] = 100;

        QJsonObject outer{};
        outer["nested"_L1] = inner;
        outer["name"_L1] = "test"_L1;

        WHEN("Converting to nlohmann::json") {
            auto result = InjaBridge::toInjaJson(outer);

            THEN("The result preserves the nested structure") {
                REQUIRE(result.is_object());
                REQUIRE(result["nested"].is_object());
                REQUIRE(result["nested"]["value"].get<int>() == 100);
                REQUIRE(result["name"].get<std::string>() == "test");
            }
        }
    }
}

SCENARIO("Rendering templates with InjaBridge", "[InjaBridge][Template]") {

    GIVEN("A simple template string and data") {
        QString template_str = "Hello, {{ name }}!"_L1;
        QJsonObject data{};
        data["name"_L1] = "World"_L1;

        WHEN("Rendering the template") {
            QString result = InjaBridge::render(template_str, data);

            THEN("The template is rendered with the provided data") {
                REQUIRE(result == "Hello, World!");
            }
        }
    }

    GIVEN("A template with conditionals") {
        QString template_str = "{% if show %}Visible{% endif %}"_L1;

        WHEN("Rendering with show=true") {
            QJsonObject data_true{};
            data_true["show"_L1] = true;
            QString result_true = InjaBridge::render(template_str, data_true);

            THEN("The conditional content is shown") {
                REQUIRE(result_true == "Visible");
            }
        }

        WHEN("Rendering with show=false") {
            QJsonObject data_false{};
            data_false["show"_L1] = false;
            QString result_false = InjaBridge::render(template_str, data_false);

            THEN("The conditional content is hidden") {
                REQUIRE(result_false == "");
            }
        }
    }

    GIVEN("A template with loops") {
        QString template_str = "{% for item in items %}{{ item }} {% endfor %}"_L1;
        QJsonObject data{};
        QJsonArray items{};
        items.append("A"_L1);
        items.append("B"_L1);
        items.append("C"_L1);
        data["items"_L1] = items;

        WHEN("Rendering the template") {
            QString result = InjaBridge::render(template_str, data);

            THEN("The loop is expanded correctly") {
                REQUIRE(result == "A B C ");
            }
        }
    }
}

SCENARIO("Rendering template files with InjaBridge", "[InjaBridge][Template][File]") {

    GIVEN("A template file and data") {
        QTemporaryFile temp_file{};
        REQUIRE(temp_file.open());

        QString template_content = "Name: {{ name }}\nAge: {{ age }}"_L1;
        temp_file.write(template_content.toUtf8());
        temp_file.close();

        QJsonObject data{};
        data["name"_L1] = "Alice"_L1;
        data["age"_L1] = 30;

        WHEN("Rendering the template file") {
            QString result = InjaBridge::renderFile(temp_file.fileName(), data);

            THEN("The template file is rendered with the provided data") {
                // Whole-number doubles are converted to int64_t so that
                // template output renders them as integers (e.g., "30" not "30.0")
                REQUIRE(result == "Name: Alice\nAge: 30");
            }
        }
    }
}

