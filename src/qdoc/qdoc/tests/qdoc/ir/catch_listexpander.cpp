// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/catalogentry.h>
#include <qdoc/ir/contentblock.h>
#include <qdoc/ir/listexpander.h>
#include <qdoc/ir/listplaceholder.h>

#include <QtCore/QJsonObject>
#include <QtCore/QList>
#include <QtCore/QString>

#include <optional>

using namespace Qt::Literals::StringLiterals;

namespace {

IR::ContentBlock makePlaceholder(const QString &variant, const QString &argument,
                                 const QString &sort = u"ascending"_s,
                                 const QString &rootName = {})
{
    IR::ContentBlock block;
    block.type = IR::BlockType::ListPlaceholder;
    block.attributes["variant"_L1] = variant;
    block.attributes["argument"_L1] = argument;
    block.attributes["sort"_L1] = sort;
    if (!rootName.isEmpty())
        block.attributes["rootName"_L1] = rootName;
    return block;
}

IR::CatalogEntry makeEntry(const QString &name, const QString &href,
                           const QString &brief = {})
{
    IR::CatalogEntry entry;
    entry.name = name;
    entry.href = href;
    entry.brief = brief;
    return entry;
}

IR::ListExpanderCallbacks callbacksReturning(
        QList<IR::CatalogEntry> cppClasses = {},
        QList<IR::CatalogEntryGroup> examples = {},
        QList<IR::CatalogEntry> compact = {},
        QList<IR::CatalogEntry> groupMembers = {})
{
    IR::ListExpanderCallbacks cb;
    cb.collectCppClasses = [cppClasses](const Node *, Qt::SortOrder) {
        return cppClasses;
    };
    cb.collectExamplesGrouped = [examples](const Node *) {
        return examples;
    };
    cb.collectCompactClasses = [compact](const Node *, const QString &) {
        return compact;
    };
    cb.collectGroupMembers = [groupMembers](const Node *, const QString &,
                                            Qt::SortOrder) {
        return groupMembers;
    };
    return cb;
}

} // namespace

SCENARIO("ListExpander replaces annotated-classes placeholder with a Catalog "
         "containing an annotated Table",
         "[IR::ListExpander][IR]")
{
    GIVEN("A body with one annotated-classes placeholder and two canned entries")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s));

        auto cb = callbacksReturning(
                { makeEntry(u"AlphaWidget"_s, u"alpha-widget.html"_s, u"First"_s),
                  makeEntry(u"BetaWidget"_s, u"beta-widget.html"_s, u"Second"_s) });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The placeholder is replaced by a Catalog block")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Catalog);
                REQUIRE(body[0].attributes["variant"_L1].toString()
                        == u"annotated-classes"_s);
            }

            THEN("The Catalog has one child Table with two rows")
            {
                REQUIRE(body[0].children.size() == 1);
                const auto &table = body[0].children[0];
                REQUIRE(table.type == IR::BlockType::Table);
                REQUIRE(table.children.size() == 2);
                REQUIRE(table.children[0].type == IR::BlockType::TableRow);
            }
        }
    }
}

SCENARIO("ListExpander passes descending sort through to the callback",
         "[IR::ListExpander][IR]")
{
    GIVEN("A descending annotated-classes placeholder")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s,
                                    u"descending"_s));

        Qt::SortOrder receivedOrder = Qt::AscendingOrder;
        IR::ListExpanderCallbacks cb;
        cb.collectCppClasses = [&receivedOrder](const Node *, Qt::SortOrder so) {
            receivedOrder = so;
            return QList<IR::CatalogEntry>{ makeEntry(u"X"_s, u"x.html"_s) };
        };

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The callback received Qt::DescendingOrder")
            {
                REQUIRE(receivedOrder == Qt::DescendingOrder);
            }
        }
    }
}

SCENARIO("ListExpander removes placeholder and emits no Catalog when the "
         "enumeration returns empty",
         "[IR::ListExpander][IR]")
{
    GIVEN("An annotated-classes placeholder and a callback returning an empty list")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s));

        auto cb = callbacksReturning({});

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The body is empty (placeholder removed, no Catalog emitted)")
            {
                REQUIRE(body.isEmpty());
            }
        }
    }
}

SCENARIO("ListExpander invokes onEmpty with the argument and variant for "
         "empty enumerations",
         "[IR::ListExpander][IR]")
{
    GIVEN("An annotated-classes placeholder whose callback returns empty")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"empty-classes"_s));

        QString receivedArgument;
        std::optional<IR::ListPlaceholderVariant> receivedVariant;
        auto cb = callbacksReturning({});
        cb.onEmpty = [&](const QString &argument,
                         IR::ListPlaceholderVariant variant) {
            receivedArgument = argument;
            receivedVariant = variant;
        };

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The placeholder is removed")
            {
                REQUIRE(body.isEmpty());
            }
            THEN("onEmpty was called with the placeholder's argument and "
                 "AnnotatedClasses variant")
            {
                REQUIRE(receivedArgument == u"empty-classes"_s);
                REQUIRE(receivedVariant.has_value());
                REQUIRE(*receivedVariant
                        == IR::ListPlaceholderVariant::AnnotatedClasses);
            }
        }
    }
}

SCENARIO("ListExpander leaves non-placeholder blocks unchanged",
         "[IR::ListExpander][IR]")
{
    GIVEN("A body with a Paragraph and no placeholders")
    {
        QList<IR::ContentBlock> body;
        IR::ContentBlock para;
        para.type = IR::BlockType::Paragraph;
        IR::InlineContent text;
        text.type = IR::InlineType::Text;
        text.text = u"unchanged"_s;
        para.inlineContent.append(text);
        body.append(para);

        auto cb = callbacksReturning();

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The paragraph is unchanged")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Paragraph);
                REQUIRE(body[0].inlineContent[0].text == u"unchanged"_s);
            }
        }
    }
}

SCENARIO("ListExpander recurses into Section children to expand nested "
         "placeholders",
         "[IR::ListExpander][IR]")
{
    GIVEN("A Section containing a placeholder among its children")
    {
        IR::ContentBlock section;
        section.type = IR::BlockType::Section;
        section.children.append(makePlaceholder(u"annotated-classes"_s,
                                                u"annotatedclasses"_s));

        QList<IR::ContentBlock> body;
        body.append(section);

        auto cb = callbacksReturning({ makeEntry(u"X"_s, u"x.html"_s) });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The nested placeholder became a Catalog inside the Section")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Section);
                REQUIRE(body[0].children.size() == 1);
                REQUIRE(body[0].children[0].type == IR::BlockType::Catalog);
            }
        }
    }
}

SCENARIO("ListExpander removes placeholders with unknown variants",
         "[IR::ListExpander][IR]")
{
    GIVEN("A placeholder with a variant no branch recognizes")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"no-such-variant"_s, u"anything"_s));

        auto cb = callbacksReturning();

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The placeholder is removed without emitting a Catalog")
            {
                REQUIRE(body.isEmpty());
            }
        }
    }
}

SCENARIO("ListExpander handles multiple placeholders in one body independently",
         "[IR::ListExpander][IR]")
{
    GIVEN("Two annotated-classes placeholders separated by a Paragraph")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s));

        IR::ContentBlock para;
        para.type = IR::BlockType::Paragraph;
        body.append(para);

        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s));

        auto cb = callbacksReturning({ makeEntry(u"X"_s, u"x.html"_s) });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("Both placeholders became Catalogs, Paragraph preserved")
            {
                REQUIRE(body.size() == 3);
                REQUIRE(body[0].type == IR::BlockType::Catalog);
                REQUIRE(body[1].type == IR::BlockType::Paragraph);
                REQUIRE(body[2].type == IR::BlockType::Catalog);
            }
        }
    }
}

SCENARIO("ListExpander produces a Link inline for entries with a non-empty href",
         "[IR::ListExpander][IR]")
{
    GIVEN("An annotated-classes placeholder with one entry")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s));

        auto cb = callbacksReturning(
                { makeEntry(u"AlphaWidget"_s, u"alpha-widget.html"_s,
                            u"desc"_s) });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The name cell holds a Link to the entry's href whose text "
                 "child is the entry name, and the brief cell holds plain text")
            {
                const auto &table = body[0].children[0];
                REQUIRE(table.children.size() == 1);
                const auto &row = table.children[0];
                REQUIRE(row.children.size() == 2);

                const auto &nameCell = row.children[0];
                REQUIRE(nameCell.inlineContent.size() == 1);
                const auto &link = nameCell.inlineContent[0];
                REQUIRE(link.type == IR::InlineType::Link);
                REQUIRE(link.href == u"alpha-widget.html"_s);
                REQUIRE(link.link.has_value());
                REQUIRE(link.link->origin == IR::LinkOrigin::Explicit);
                REQUIRE(link.link->state == IR::LinkState::Resolved);
                REQUIRE(link.children.size() == 1);
                REQUIRE(link.children[0].type == IR::InlineType::Text);
                REQUIRE(link.children[0].text == u"AlphaWidget"_s);

                const auto &briefCell = row.children[1];
                REQUIRE(briefCell.inlineContent.size() == 1);
                REQUIRE(briefCell.inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(briefCell.inlineContent[0].text == u"desc"_s);
            }
        }
    }
}

SCENARIO("ListExpander surfaces since and isDeprecated as row attributes",
         "[IR::ListExpander][IR]")
{
    GIVEN("Two entries: one with a since stamp, one deprecated, plus a plain entry")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-classes"_s, u"annotatedclasses"_s));

        IR::CatalogEntry sinceEntry =
                makeEntry(u"Versioned"_s, u"versioned.html"_s);
        sinceEntry.since = u"6.7"_s;

        IR::CatalogEntry deprecated =
                makeEntry(u"OldThing"_s, u"old-thing.html"_s);
        deprecated.isDeprecated = true;

        IR::CatalogEntry plain = makeEntry(u"Plain"_s, u"plain.html"_s);

        auto cb = callbacksReturning({ sinceEntry, deprecated, plain });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("Each row carries the metadata the source extracted, "
                 "and rows for entries without metadata omit the keys")
            {
                const auto &rows = body[0].children[0].children;
                REQUIRE(rows.size() == 3);

                REQUIRE(rows[0].attributes["since"_L1].toString()
                        == u"6.7"_s);
                REQUIRE_FALSE(rows[0].attributes.contains("deprecated"_L1));

                REQUIRE_FALSE(rows[1].attributes.contains("since"_L1));
                REQUIRE(rows[1].attributes["deprecated"_L1].toBool() == true);

                REQUIRE_FALSE(rows[2].attributes.contains("since"_L1));
                REQUIRE_FALSE(rows[2].attributes.contains("deprecated"_L1));
            }
        }
    }
}

SCENARIO("ListExpander renders annotated-examples groups as heading + table pairs",
         "[IR::ListExpander][IR]")
{
    GIVEN("Two example groups")
    {
        IR::CatalogEntryGroup g1;
        g1.label = u"Qt Core"_s;
        g1.anchorId = u"qt-core"_s;
        g1.entries.append(makeEntry(u"Hello"_s, u"hello.html"_s));

        IR::CatalogEntryGroup g2;
        g2.label = u"Qt Quick"_s;
        g2.anchorId = u"qt-quick"_s;
        g2.entries.append(makeEntry(u"Clocks"_s, u"clocks.html"_s));
        g2.entries.append(makeEntry(u"Game"_s, u"game.html"_s));

        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-examples"_s, u"annotatedexamples"_s));

        auto cb = callbacksReturning({}, { g1, g2 });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The Catalog has four children: heading, table, heading, table")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Catalog);
                REQUIRE(body[0].children.size() == 4);
                REQUIRE(body[0].children[0].type == IR::BlockType::SectionHeading);
                REQUIRE(body[0].children[1].type == IR::BlockType::Table);
                REQUIRE(body[0].children[2].type == IR::BlockType::SectionHeading);
                REQUIRE(body[0].children[3].type == IR::BlockType::Table);
            }

            THEN("The first heading carries the first group's label")
            {
                const auto &h = body[0].children[0];
                REQUIRE(h.inlineContent.size() == 1);
                REQUIRE(h.inlineContent[0].text == u"Qt Core"_s);
                REQUIRE(h.attributes["id"_L1].toString() == u"qt-core"_s);
            }

            THEN("The second table holds two rows")
            {
                REQUIRE(body[0].children[3].children.size() == 2);
            }
        }
    }
}

SCENARIO("ListExpander suppresses the heading for an empty-label "
         "annotated-examples group",
         "[IR::ListExpander][IR]")
{
    GIVEN("A single group with an empty label and one populated entry")
    {
        IR::CatalogEntryGroup unlabeled;
        unlabeled.label = QString();
        unlabeled.anchorId = QString();
        unlabeled.entries.append(makeEntry(u"Hello"_s, u"hello.html"_s));

        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-examples"_s,
                                    u"annotatedexamples"_s));

        auto cb = callbacksReturning({}, { unlabeled });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The Catalog carries only the table — no SectionHeading "
                 "child is emitted")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Catalog);
                REQUIRE(body[0].children.size() == 1);
                REQUIRE(body[0].children[0].type == IR::BlockType::Table);
            }
        }
    }
}

SCENARIO("ListExpander emits headings only for labeled groups when groups "
         "are mixed in annotated-examples",
         "[IR::ListExpander][IR]")
{
    GIVEN("Two groups: one labeled, one unlabeled, both with entries")
    {
        IR::CatalogEntryGroup labeled;
        labeled.label = u"Qt Core"_s;
        labeled.anchorId = u"qt-core"_s;
        labeled.entries.append(makeEntry(u"Hello"_s, u"hello.html"_s));

        IR::CatalogEntryGroup unlabeled;
        unlabeled.label = QString();
        unlabeled.anchorId = QString();
        unlabeled.entries.append(makeEntry(u"Anonymous"_s,
                                           u"anonymous.html"_s));

        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-examples"_s,
                                    u"annotatedexamples"_s));

        auto cb = callbacksReturning({}, { labeled, unlabeled });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The Catalog has three children: heading, table, table")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].children.size() == 3);
                REQUIRE(body[0].children[0].type == IR::BlockType::SectionHeading);
                REQUIRE(body[0].children[1].type == IR::BlockType::Table);
                REQUIRE(body[0].children[2].type == IR::BlockType::Table);
            }

            THEN("The single heading carries the labeled group's text")
            {
                const auto &h = body[0].children[0];
                REQUIRE(h.inlineContent.size() == 1);
                REQUIRE(h.inlineContent[0].text == u"Qt Core"_s);
            }
        }
    }
}

SCENARIO("ListExpander renders compact-classes as a bullet list",
         "[IR::ListExpander][IR]")
{
    GIVEN("A compact-classes placeholder with three entries")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"compact-classes"_s, u"classes"_s));

        auto cb = callbacksReturning({}, {},
                { makeEntry(u"A"_s, u"a.html"_s),
                  makeEntry(u"B"_s, u"b.html"_s),
                  makeEntry(u"C"_s, u"c.html"_s) });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The Catalog wraps a List with three ListItem children")
            {
                REQUIRE(body.size() == 1);
                const auto &cat = body[0];
                REQUIRE(cat.type == IR::BlockType::Catalog);
                REQUIRE(cat.children.size() == 1);
                REQUIRE(cat.children[0].type == IR::BlockType::List);
                REQUIRE(cat.children[0].children.size() == 3);
                REQUIRE(cat.children[0].children[0].type == IR::BlockType::ListItem);
            }
        }
    }
}

SCENARIO("ListExpander forwards rootName to the compact-classes callback",
         "[IR::ListExpander][IR]")
{
    GIVEN("A compact-classes placeholder with rootName=QAbstract")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"compact-classes"_s, u"classes QAbstract"_s,
                                    u"ascending"_s, u"QAbstract"_s));

        QString receivedRoot;
        IR::ListExpanderCallbacks cb;
        cb.collectCompactClasses = [&receivedRoot](const Node *, const QString &r) {
            receivedRoot = r;
            return QList<IR::CatalogEntry>{ makeEntry(u"QAbstractButton"_s,
                                                      u"qabstractbutton.html"_s) };
        };

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The callback received rootName=QAbstract")
            {
                REQUIRE(receivedRoot == u"QAbstract"_s);
            }
        }
    }
}

SCENARIO("ListExpander renders annotated-group with the group argument preserved",
         "[IR::ListExpander][IR]")
{
    GIVEN("An annotated-group placeholder with argument=my-group")
    {
        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-group"_s, u"my-group"_s));

        auto cb = callbacksReturning({}, {}, {},
                { makeEntry(u"Entry1"_s, u"e1.html"_s) });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The Catalog carries variant=annotated-group and argument=my-group")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Catalog);
                REQUIRE(body[0].attributes["variant"_L1].toString()
                        == u"annotated-group"_s);
                REQUIRE(body[0].attributes["argument"_L1].toString()
                        == u"my-group"_s);
            }
        }
    }
}

SCENARIO("ListExpander skips empty groups inside annotated-examples and "
         "drops the placeholder when every group came in empty",
         "[IR::ListExpander][IR]")
{
    GIVEN("Two groups: one with an entry, one pre-emptied")
    {
        IR::CatalogEntryGroup populated;
        populated.label = u"Qt Core"_s;
        populated.anchorId = u"qt-core"_s;
        populated.entries.append(makeEntry(u"Hello"_s, u"hello.html"_s));

        IR::CatalogEntryGroup empty;
        empty.label = u"Qt Empty"_s;
        empty.anchorId = u"qt-empty"_s;

        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-examples"_s,
                                    u"annotatedexamples"_s));

        auto cb = callbacksReturning({}, { populated, empty });

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("Only the populated group emits a heading + table pair")
            {
                REQUIRE(body.size() == 1);
                REQUIRE(body[0].type == IR::BlockType::Catalog);
                REQUIRE(body[0].children.size() == 2);
                REQUIRE(body[0].children[0].type == IR::BlockType::SectionHeading);
                REQUIRE(body[0].children[1].type == IR::BlockType::Table);
            }
        }
    }

    GIVEN("A single, pre-emptied group and an onEmpty hook")
    {
        IR::CatalogEntryGroup empty;
        empty.label = u"Qt Empty"_s;

        QList<IR::ContentBlock> body;
        body.append(makePlaceholder(u"annotated-examples"_s,
                                    u"annotatedexamples"_s));

        bool warned = false;
        auto cb = callbacksReturning({}, { empty });
        cb.onEmpty = [&warned](const QString &, IR::ListPlaceholderVariant) {
            warned = true;
        };

        WHEN("expand runs")
        {
            IR::ListExpander expander(std::move(cb));
            expander.expand(body, nullptr);

            THEN("The placeholder is dropped and onEmpty is invoked")
            {
                REQUIRE(body.isEmpty());
                REQUIRE(warned);
            }
        }
    }
}
