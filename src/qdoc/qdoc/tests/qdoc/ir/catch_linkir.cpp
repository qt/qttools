// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/linkir.h>

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("LinkIR basic structure", "[LinkIR][IR]") {

    GIVEN("A default-constructed LinkIR") {
        LinkIR link;

        THEN("The link is not valid when target is empty") {
            REQUIRE(!link.isValid());
        }

        THEN("The default state is Resolved") {
            REQUIRE(link.state == LinkIR::State::Resolved);
            REQUIRE(link.isResolved());
            REQUIRE(!link.isExternal());
        }
    }

    GIVEN("A LinkIR with target and text") {
        LinkIR link;
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

    GIVEN("A LinkIR with all fields populated") {
        LinkIR link;
        link.target = "https://doc.qt.io/qt-6/qstring.html"_L1;
        link.text = "QString"_L1;
        link.title = "Qt 6 QString Documentation"_L1;
        link.state = LinkIR::State::External;
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

SCENARIO("LinkIR state handling", "[LinkIR][IR][State]") {

    GIVEN("A LinkIR with Resolved state") {
        LinkIR link;
        link.target = "page.html"_L1;
        link.text = "Page"_L1;
        link.state = LinkIR::State::Resolved;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("State string is 'resolved'") {
                REQUIRE(json["state"_L1].toString() == "resolved");
            }
        }
    }

    GIVEN("A LinkIR with External state") {
        LinkIR link;
        link.target = "https://example.com"_L1;
        link.text = "Example"_L1;
        link.state = LinkIR::State::External;

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

    GIVEN("A LinkIR with Unresolved state") {
        LinkIR link;
        link.text = "MissingClass"_L1;
        link.state = LinkIR::State::Unresolved;
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

    GIVEN("A LinkIR with Broken state") {
        LinkIR link;
        link.target = "#removed-section"_L1;
        link.text = "Removed Section"_L1;
        link.state = LinkIR::State::Broken;

        WHEN("Converting to JSON") {
            QJsonObject json = link.toJson();

            THEN("State string is 'broken'") {
                REQUIRE(json["state"_L1].toString() == "broken");
            }
        }
    }
}

SCENARIO("LinkIR originalTarget tracking", "[LinkIR][IR][Diagnostics]") {

    GIVEN("A LinkIR where target matches originalTarget") {
        LinkIR link;
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

    GIVEN("A LinkIR where target differs from originalTarget") {
        LinkIR link;
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

SCENARIO("LinkIR template usage patterns", "[LinkIR][IR][Integration]") {

    GIVEN("A typical internal link for template rendering") {
        LinkIR link;
        link.target = "qtcore-module.html"_L1;
        link.text = "Qt Core"_L1;
        link.state = LinkIR::State::Resolved;

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
        LinkIR link;
        link.target = "https://doc.qt.io/"_L1;
        link.text = "Qt Documentation"_L1;
        link.title = "Opens in new window"_L1;
        link.state = LinkIR::State::External;

        WHEN("Converting to JSON for template") {
            QJsonObject json = link.toJson();

            THEN("Template can add title attribute for accessibility") {
                // Template: <a href="{{ link.target }}" title="{{ link.title }}">
                REQUIRE(json["title"_L1].toString() == "Opens in new window");
            }
        }
    }
}

