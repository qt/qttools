// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/atomcontext.h>

#include <QJsonObject>
#include <QJsonValue>

using namespace Qt::Literals::StringLiterals;
using ContextType = IR::AtomContext::ContextType;

SCENARIO("AtomContext default construction", "[IR::AtomContext][IR]") {

    GIVEN("A default-constructed AtomContext") {
        IR::AtomContext ctx;

        THEN("The stack is empty") {
            REQUIRE(ctx.isEmpty());
        }

        THEN("The depth is zero") {
            REQUIRE(ctx.depth() == 0);
        }
    }
}

SCENARIO("AtomContext push and pop basics", "[IR::AtomContext][IR]") {

    GIVEN("An empty AtomContext") {
        IR::AtomContext ctx;

        WHEN("A Paragraph frame is pushed") {
            ctx.push(ContextType::Paragraph);

            THEN("The context is no longer empty") {
                REQUIRE_FALSE(ctx.isEmpty());
            }

            THEN("The depth is 1") {
                REQUIRE(ctx.depth() == 1);
            }

            THEN("The current frame type is Paragraph") {
                REQUIRE(ctx.current().type == ContextType::Paragraph);
            }

            AND_WHEN("The frame is popped") {
                auto frame = ctx.pop();

                THEN("The popped frame type is Paragraph") {
                    REQUIRE(frame.type == ContextType::Paragraph);
                }

                THEN("The context is empty again") {
                    REQUIRE(ctx.isEmpty());
                }

                THEN("The depth is zero again") {
                    REQUIRE(ctx.depth() == 0);
                }
            }
        }
    }
}

SCENARIO("AtomContext isInContext queries", "[IR::AtomContext][IR]") {

    GIVEN("An AtomContext with List, ListItem, and Paragraph pushed") {
        IR::AtomContext ctx;
        ctx.push(ContextType::List);
        ctx.push(ContextType::ListItem);
        ctx.push(ContextType::Paragraph);

        THEN("isInContext returns true for all pushed types") {
            REQUIRE(ctx.isInContext(ContextType::List));
            REQUIRE(ctx.isInContext(ContextType::ListItem));
            REQUIRE(ctx.isInContext(ContextType::Paragraph));
        }

        THEN("isInContext returns false for types not in the stack") {
            REQUIRE_FALSE(ctx.isInContext(ContextType::CodeBlock));
            REQUIRE_FALSE(ctx.isInContext(ContextType::Table));
            REQUIRE_FALSE(ctx.isInContext(ContextType::Section));
        }

        WHEN("Paragraph is popped") {
            ctx.pop();

            THEN("isInContext returns false for Paragraph") {
                REQUIRE_FALSE(ctx.isInContext(ContextType::Paragraph));
            }

            THEN("isInContext still returns true for List and ListItem") {
                REQUIRE(ctx.isInContext(ContextType::List));
                REQUIRE(ctx.isInContext(ContextType::ListItem));
            }
        }
    }
}

SCENARIO("AtomContext nested lists", "[IR::AtomContext][IR]") {

    GIVEN("A nested list structure: List > ListItem > List > ListItem") {
        IR::AtomContext ctx;
        ctx.push(ContextType::List);
        ctx.push(ContextType::ListItem);
        ctx.push(ContextType::List);
        ctx.push(ContextType::ListItem);

        THEN("The depth is 4") {
            REQUIRE(ctx.depth() == 4);
        }

        THEN("The current frame is a ListItem") {
            REQUIRE(ctx.current().type == ContextType::ListItem);
        }

        WHEN("Two frames are popped (inner list removed)") {
            ctx.pop();
            ctx.pop();

            THEN("The depth is 2") {
                REQUIRE(ctx.depth() == 2);
            }

            THEN("The current frame is the outer ListItem") {
                REQUIRE(ctx.current().type == ContextType::ListItem);
            }

            THEN("isInContext still reports List") {
                REQUIRE(ctx.isInContext(ContextType::List));
            }
        }
    }
}

SCENARIO("AtomContext frame attributes", "[IR::AtomContext][IR]") {

    GIVEN("A CodeBlock frame pushed with a language attribute") {
        IR::AtomContext ctx;
        QJsonObject attrs;
        attrs["language"_L1] = "cpp"_L1;
        ctx.push(ContextType::CodeBlock, attrs);

        THEN("The current frame has the language attribute") {
            REQUIRE(ctx.current().attributes["language"_L1].toString() == "cpp"_L1);
        }

        THEN("The current frame type is CodeBlock") {
            REQUIRE(ctx.current().type == ContextType::CodeBlock);
        }
    }
}

SCENARIO("AtomContext section nesting", "[IR::AtomContext][IR]") {

    GIVEN("A Section with a SectionHeading containing a level attribute") {
        IR::AtomContext ctx;
        ctx.push(ContextType::Section);

        QJsonObject headingAttrs;
        headingAttrs["level"_L1] = 2;
        ctx.push(ContextType::SectionHeading, headingAttrs);

        THEN("The heading level attribute is accessible") {
            REQUIRE(ctx.current().attributes["level"_L1].toInt() == 2);
        }

        THEN("The current frame type is SectionHeading") {
            REQUIRE(ctx.current().type == ContextType::SectionHeading);
        }

        WHEN("The heading is popped and Paragraph is pushed") {
            ctx.pop();
            ctx.push(ContextType::Paragraph);

            THEN("isInContext reports Section") {
                REQUIRE(ctx.isInContext(ContextType::Section));
            }

            THEN("isInContext does not report SectionHeading") {
                REQUIRE_FALSE(ctx.isInContext(ContextType::SectionHeading));
            }

            THEN("The current frame is Paragraph") {
                REQUIRE(ctx.current().type == ContextType::Paragraph);
            }
        }
    }
}

SCENARIO("AtomContext clear resets all state", "[IR::AtomContext][IR]") {

    GIVEN("An AtomContext with several frames pushed") {
        IR::AtomContext ctx;
        ctx.push(ContextType::Section);
        ctx.push(ContextType::Paragraph);
        ctx.push(ContextType::List);

        REQUIRE(ctx.depth() == 3);

        WHEN("clear is called") {
            ctx.clear();

            THEN("The context is empty") {
                REQUIRE(ctx.isEmpty());
            }

            THEN("The depth is zero") {
                REQUIRE(ctx.depth() == 0);
            }
        }
    }
}

