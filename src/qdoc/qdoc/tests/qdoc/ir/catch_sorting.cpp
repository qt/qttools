// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/sorting.h>

#include <QtCore/QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("Sorting::parseSortOrder recognizes the descending directive",
         "[Sorting][IR]")
{
    GIVEN("The string \"descending\"")
    {
        const QString directive = u"descending"_s;

        WHEN("parseSortOrder is called")
        {
            const Qt::SortOrder order = Sorting::parseSortOrder(directive);

            THEN("The result is Qt::DescendingOrder")
            {
                REQUIRE(order == Qt::DescendingOrder);
            }
        }
    }
}

SCENARIO("Sorting::parseSortOrder returns ascending for the ascending directive",
         "[Sorting][IR]")
{
    GIVEN("The string \"ascending\"")
    {
        const QString directive = u"ascending"_s;

        WHEN("parseSortOrder is called")
        {
            const Qt::SortOrder order = Sorting::parseSortOrder(directive);

            THEN("The result is Qt::AscendingOrder")
            {
                REQUIRE(order == Qt::AscendingOrder);
            }
        }
    }
}

SCENARIO("Sorting::parseSortOrder defaults to ascending for the empty directive",
         "[Sorting][IR]")
{
    GIVEN("An empty string")
    {
        const QString directive;

        WHEN("parseSortOrder is called")
        {
            const Qt::SortOrder order = Sorting::parseSortOrder(directive);

            THEN("The result is Qt::AscendingOrder (legacy parity)")
            {
                REQUIRE(order == Qt::AscendingOrder);
            }
        }
    }
}

SCENARIO("Sorting::parseSortOrder defaults to ascending for unknown directives",
         "[Sorting][IR]")
{
    GIVEN("A string that is neither \"descending\" nor \"ascending\"")
    {
        const QString directive = u"annotatedclasses"_s;

        WHEN("parseSortOrder is called")
        {
            const Qt::SortOrder order = Sorting::parseSortOrder(directive);

            THEN("The result is Qt::AscendingOrder (matches Generator::sortOrder falling through)")
            {
                REQUIRE(order == Qt::AscendingOrder);
            }
        }
    }
}
