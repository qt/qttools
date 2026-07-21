// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>
#include "qdoc/textutils.h"

QT_USE_NAMESPACE

SCENARIO("Detecting whether text ends with sentence-ending punctuation",
         "[TextUtils][endsWithSentenceTerminator]")
{
    GIVEN("A non-empty string that does not end in terminating punctuation") {
        THEN("It is reported as unterminated") {
            REQUIRE_FALSE(TextUtils::endsWithSentenceTerminator(
                    u"Use setTimeZone() instead"));
        }
    }

    GIVEN("A string that ends with a full stop") {
        THEN("It is reported as terminated") {
            REQUIRE(TextUtils::endsWithSentenceTerminator(
                    u"Use setTimeZone() instead."));
        }
    }

    GIVEN("A string that ends with a question mark") {
        THEN("It is reported as terminated") {
            REQUIRE(TextUtils::endsWithSentenceTerminator(u"Really?"));
        }
    }

    GIVEN("A string that ends with an exclamation mark") {
        THEN("It is reported as terminated") {
            REQUIRE(TextUtils::endsWithSentenceTerminator(u"Stop!"));
        }
    }

    GIVEN("An empty string") {
        THEN("It raises no complaint") {
            REQUIRE(TextUtils::endsWithSentenceTerminator(u""));
        }
    }

    GIVEN("A whitespace-only string") {
        THEN("It raises no complaint") {
            REQUIRE(TextUtils::endsWithSentenceTerminator(u"   "));
        }
    }

    GIVEN("A terminated string with trailing whitespace") {
        THEN("The trailing whitespace is tolerated") {
            REQUIRE(TextUtils::endsWithSentenceTerminator(
                    u"trailing dot then spaces.   "));
        }
    }
}
