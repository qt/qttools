// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/link.h>

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("IR::Link basic structure", "[IR::Link][IR]") {

    GIVEN("A default-constructed IR::Link") {
        IR::Link link;

        THEN("The link is not valid when target is empty") {
            REQUIRE(!link.isValid());
        }

        THEN("The default state is Resolved") {
            REQUIRE(link.state == IR::Link::State::Resolved);
            REQUIRE(link.isResolved());
            REQUIRE(!link.isExternal());
        }
    }

    GIVEN("A IR::Link with target and text") {
        IR::Link link;
        link.target = "qstring.html"_L1;
        link.text = "QString"_L1;

        THEN("The link is valid") {
            REQUIRE(link.isValid());
        }

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("The JSON contains target and text") {
                REQUIRE(json["target"_L1].toString() == "qstring.html");
                REQUIRE(json["text"_L1].toString() == "QString");
            }

            THEN("The state is represented as a string") {
                REQUIRE(json["state"_L1].toString() == "resolved");
            }

            THEN("Helper boolean fields are present") {
                REQUIRE(json["isResolved"_L1].toBool() == true);
                REQUIRE(json["isExternal"_L1].toBool() == false);
            }

            THEN("Optional title is not present when empty") {
                REQUIRE(!json.contains("title"_L1));
            }

            THEN("originalTarget is not present when same as target") {
                REQUIRE(!json.contains("originalTarget"_L1));
            }
        }
    }

    GIVEN("A IR::Link with all fields populated") {
        IR::Link link;
        link.target = "https://doc.qt.io/qt-6/qstring.html"_L1;
        link.text = "QString"_L1;
        link.title = "Qt 6 QString Documentation"_L1;
        link.state = IR::Link::State::External;
        link.originalTarget = "QString"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("All fields are present") {
                REQUIRE(json["target"_L1].toString() == "https://doc.qt.io/qt-6/qstring.html");
                REQUIRE(json["text"_L1].toString() == "QString");
                REQUIRE(json["title"_L1].toString() == "Qt 6 QString Documentation");
                REQUIRE(json["state"_L1].toString() == "external");
                REQUIRE(json["originalTarget"_L1].toString() == "QString");
            }

            THEN("External state is correctly reflected in helpers") {
                REQUIRE(json["isResolved"_L1].toBool() == false);
                REQUIRE(json["isExternal"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("IR::Link state handling", "[IR::Link][IR][State]") {

    GIVEN("A IR::Link with Resolved state") {
        IR::Link link;
        link.target = "page.html"_L1;
        link.text = "Page"_L1;
        link.state = IR::Link::State::Resolved;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("State string is 'resolved'") {
                REQUIRE(json["state"_L1].toString() == "resolved");
            }
        }
    }

    GIVEN("A IR::Link with External state") {
        IR::Link link;
        link.target = "https://example.com"_L1;
        link.text = "Example"_L1;
        link.state = IR::Link::State::External;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("State string is 'external'") {
                REQUIRE(json["state"_L1].toString() == "external");
            }
        }

        THEN("isExternal() returns true") {
            REQUIRE(link.isExternal());
        }
    }

    GIVEN("A IR::Link with Unresolved state") {
        IR::Link link;
        link.text = "MissingClass"_L1;
        link.state = IR::Link::State::Unresolved;
        link.originalTarget = "MissingClass"_L1;

        THEN("The link is still valid for template rendering") {
            // Unresolved links should still be renderable (perhaps with special styling)
            REQUIRE(link.isValid());
        }

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("State string is 'unresolved'") {
                REQUIRE(json["state"_L1].toString() == "unresolved");
            }

            THEN("isResolved is false") {
                REQUIRE(json["isResolved"_L1].toBool() == false);
            }
        }
    }

    GIVEN("A IR::Link with Broken state") {
        IR::Link link;
        link.target = "#removed-section"_L1;
        link.text = "Removed Section"_L1;
        link.state = IR::Link::State::Broken;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("State string is 'broken'") {
                REQUIRE(json["state"_L1].toString() == "broken");
            }
        }
    }
}

SCENARIO("IR::Link originalTarget tracking", "[IR::Link][IR][Diagnostics]") {

    GIVEN("A IR::Link where target matches originalTarget") {
        IR::Link link;
        link.target = "qstring.html"_L1;
        link.text = "QString"_L1;
        link.originalTarget = "qstring.html"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("originalTarget is not included (redundant)") {
                REQUIRE(!json.contains("originalTarget"_L1));
            }
        }
    }

    GIVEN("A IR::Link where target differs from originalTarget") {
        IR::Link link;
        link.target = "qstring.html#section"_L1;
        link.text = "QString::section()"_L1;
        link.originalTarget = "QString::section()"_L1;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("originalTarget is included for diagnostics") {
                REQUIRE(json.contains("originalTarget"_L1));
                REQUIRE(json["originalTarget"_L1].toString() == "QString::section()");
            }
        }
    }
}

SCENARIO("IR::Link template usage patterns", "[IR::Link][IR][Integration]") {

    GIVEN("A typical internal link for template rendering") {
        IR::Link link;
        link.target = "qtcore-module.html"_L1;
        link.text = "Qt Core"_L1;
        link.state = IR::Link::State::Resolved;

        WHEN("Converting to JSON for template") {
            QJsonObject json = link.toJson();

            THEN("Template can use target for href attribute") {
                // Template: <a href="{{ link.target }}">{{ link.text }}</a>
                REQUIRE(!json["target"_L1].toString().isEmpty());
                REQUIRE(!json["text"_L1].toString().isEmpty());
            }

            THEN("Template can conditionally style based on state") {
                // Template: {% if link.isExternal %}class="external"{% endif %}
                REQUIRE(json.contains("isExternal"_L1));
                REQUIRE(json["isExternal"_L1].isBool());
            }
        }
    }

    GIVEN("An external link with title for accessibility") {
        IR::Link link;
        link.target = "https://doc.qt.io/"_L1;
        link.text = "Qt Documentation"_L1;
        link.title = "Opens in new window"_L1;
        link.state = IR::Link::State::External;

        WHEN("Converting to JSON for template") {
            QJsonObject json = link.toJson();

            THEN("Template can add title attribute for accessibility") {
                // Template: <a href="{{ link.target }}" title="{{ link.title }}">
                REQUIRE(json["title"_L1].toString() == "Opens in new window");
            }
        }
    }
}

