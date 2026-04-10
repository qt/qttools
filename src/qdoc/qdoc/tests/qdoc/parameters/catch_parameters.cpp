// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>
#include <qdoc/parameters.h>

using namespace Qt::Literals::StringLiterals;

QT_USE_NAMESPACE

TEST_CASE("Extract a trailing parameter list without losing nested parentheses", "[Parameters]")
{
    SECTION("An empty list is distinct from no list") {
        QString name = u"Item.method()"_s;
        const QString parameters = Parameters::extractParameters(name);
        REQUIRE(name == u"Item.method"_s);
        REQUIRE(parameters.isEmpty());
        REQUIRE(!parameters.isNull());
    }

    SECTION("Nested declarators stay inside the parameter list") {
        QString name = u"Item.method(void (*)(int, int), Other.Type)"_s;
        const QString parameters = Parameters::extractParameters(name);
        REQUIRE(name == u"Item.method"_s);
        REQUIRE(parameters == u"void (*)(int, int), Other.Type"_s);
    }

    SECTION("Parentheses in an operator name stay in the name") {
        QString name = u"Item::operator()(int)"_s;
        REQUIRE(Parameters::extractParameters(name) == u"int"_s);
        REQUIRE(name == u"Item::operator()"_s);
    }

    SECTION("Missing and unbalanced lists leave the target unchanged") {
        for (const QString &target : {u"Item.method"_s, u"Item.method)"_s,
                                      u"Item.method(int))"_s, u"Item.method((int)"_s,
                                      u"Item.method(int"_s}) {
            CAPTURE(target);
            QString name = target;
            REQUIRE(Parameters::extractParameters(name).isNull());
            REQUIRE(name == target);
        }
    }
}
