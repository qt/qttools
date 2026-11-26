// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch_conversions/qdoc_catch_conversions.h>

#include <catch/catch.hpp>

#include <qdoc/textutils.h>

QT_USE_NAMESPACE

using namespace Qt::StringLiterals;

SCENARIO("Cleaning anchor references", "[TextUtils][Anchor]")
{
    GIVEN("References that are already valid identifiers") {
        THEN("They pass through unchanged in both syntaxes") {
            REQUIRE(TextUtils::cleanAnchorRef(u"append"_s) == u"append"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"size-1"_s) == u"size-1"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"a.b_c"_s, TextUtils::AnchorSyntax::Xml)
                    == u"a.b_c"_s);
        }
    }

    GIVEN("A destructor and an underscore-prefixed name") {
        THEN("The leading character is spelled out") {
            REQUIRE(TextUtils::cleanAnchorRef(u"~QObject"_s) == u"dtor.QObject"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"_internal"_s) == u"underscore.internal"_s);
        }
    }

    GIVEN("Operator names") {
        THEN("Operator characters are spelled out") {
            REQUIRE(TextUtils::cleanAnchorRef(u"operator=="_s) == u"operator-eq-eq"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"operator<"_s) == u"operator-lt"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"operator!"_s) == u"operator-not"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"operator&"_s) == u"operator-and"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"operator>"_s) == u"operator-gt"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"operator()"_s) == u"operator-28-29"_s);
        }
    }

    GIVEN("A reference with whitespace and a fragment marker") {
        THEN("Whitespace becomes a hyphen and the marker is preserved") {
            REQUIRE(TextUtils::cleanAnchorRef(u"see also#top"_s) == u"see-also#top"_s);
        }
    }

    GIVEN("A reference that starts with a digit or contains a colon") {
        THEN("HTML syntax keeps the digit and encodes the colon") {
            REQUIRE(TextUtils::cleanAnchorRef(u"1st"_s) == u"1st"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"a:b"_s) == u"a-3ab"_s);
        }
        THEN("XML syntax prefixes the digit and keeps the colon") {
            REQUIRE(TextUtils::cleanAnchorRef(u"1st"_s, TextUtils::AnchorSyntax::Xml)
                    == u"A1st"_s);
            REQUIRE(TextUtils::cleanAnchorRef(u"a:b"_s, TextUtils::AnchorSyntax::Xml)
                    == u"a:b"_s);
        }
    }

    GIVEN("An empty reference") {
        THEN("The result is empty") {
            REQUIRE(TextUtils::cleanAnchorRef(QString()).isEmpty());
        }
    }
}
