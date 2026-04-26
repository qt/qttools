// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/catalogentry.h>

#include <QtCore/QString>

using namespace Qt::Literals::StringLiterals;

SCENARIO("CatalogEntry default-constructs with empty strings and non-deprecated",
         "[IR::CatalogEntry][IR]")
{
    GIVEN("A default-constructed CatalogEntry")
    {
        IR::CatalogEntry entry;

        THEN("All string fields are empty")
        {
            REQUIRE(entry.name.isEmpty());
            REQUIRE(entry.href.isEmpty());
            REQUIRE(entry.brief.isEmpty());
            REQUIRE(entry.since.isEmpty());
        }

        THEN("isDeprecated is false")
        {
            REQUIRE_FALSE(entry.isDeprecated);
        }
    }
}

SCENARIO("CatalogEntry fields accept assignment",
         "[IR::CatalogEntry][IR]")
{
    GIVEN("A CatalogEntry with populated fields")
    {
        IR::CatalogEntry entry;
        entry.name = u"AlphaWidget"_s;
        entry.href = u"alpha-widget.html"_s;
        entry.brief = u"The first widget"_s;
        entry.since = u"6.9"_s;
        entry.isDeprecated = true;

        THEN("The values round-trip")
        {
            REQUIRE(entry.name == u"AlphaWidget"_s);
            REQUIRE(entry.href == u"alpha-widget.html"_s);
            REQUIRE(entry.brief == u"The first widget"_s);
            REQUIRE(entry.since == u"6.9"_s);
            REQUIRE(entry.isDeprecated);
        }
    }
}

SCENARIO("CatalogEntryGroup default-constructs empty and accepts entries",
         "[IR::CatalogEntryGroup][IR]")
{
    GIVEN("A default-constructed CatalogEntryGroup")
    {
        IR::CatalogEntryGroup group;

        THEN("Label and anchor are empty, entries list is empty")
        {
            REQUIRE(group.label.isEmpty());
            REQUIRE(group.anchorId.isEmpty());
            REQUIRE(group.entries.isEmpty());
        }

        WHEN("Two entries are appended and the group is labeled")
        {
            group.label = u"Qt Core"_s;
            group.anchorId = u"qt-core"_s;

            IR::CatalogEntry first;
            first.name = u"Hello Example"_s;
            first.href = u"hello-example.html"_s;
            group.entries.append(first);

            IR::CatalogEntry second;
            second.name = u"World Example"_s;
            second.href = u"world-example.html"_s;
            group.entries.append(second);

            THEN("The label and entries are preserved in order")
            {
                REQUIRE(group.label == u"Qt Core"_s);
                REQUIRE(group.anchorId == u"qt-core"_s);
                REQUIRE(group.entries.size() == 2);
                REQUIRE(group.entries[0].name == u"Hello Example"_s);
                REQUIRE(group.entries[1].name == u"World Example"_s);
            }
        }
    }
}
