// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/ir/contentbuilder.h>
#include <qdoc/atom.h>

#include <QList>
#include <QString>

using namespace Qt::Literals::StringLiterals;

/*!
    Helper to build an Atom chain and manage memory.

    The first atom is stack-allocated; subsequent atoms are heap-allocated
    and owned by this struct. The chain is linked via the Atom(previous, type, string)
    constructor.
*/
struct AtomChain
{
    Atom first;
    QList<Atom *> owned;

    explicit AtomChain(Atom::AtomType type, const QString &str = {})
        : first(type, str)
    {
    }

    AtomChain(Atom::AtomType type, const QString &p1, const QString &p2)
        : first(type, p1, p2)
    {
    }

    ~AtomChain() { qDeleteAll(owned); }

    Atom *append(Atom::AtomType type, const QString &str = {})
    {
        Atom *prev = owned.isEmpty() ? &first : owned.last();
        auto *a = new Atom(prev, type, str);
        owned.append(a);
        return a;
    }

    Atom *append(Atom::AtomType type, const QString &p1, const QString &p2)
    {
        Atom *prev = owned.isEmpty() ? &first : owned.last();
        auto *a = new Atom(prev, type, p1, p2);
        owned.append(a);
        return a;
    }
};

SCENARIO("ContentBuilder with null atom returns empty list", "[IR::ContentBuilder][IR]")
{
    GIVEN("A ContentBuilder")
    {
        IR::ContentBuilder builder;

        WHEN("build() is called with nullptr")
        {
            auto blocks = builder.build(nullptr);

            THEN("The result is empty")
            {
                REQUIRE(blocks.isEmpty());
            }
        }
    }
}

SCENARIO("ContentBuilder produces a Paragraph from ParaLeft/String/ParaRight",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("An atom chain: ParaLeft -> String('Hello world') -> ParaRight")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"Hello world"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one Paragraph block")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
            }

            THEN("The paragraph has one Text inline")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[0].text == u"Hello world"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder builds nested Section with heading and paragraph",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("An atom chain: Section > SectionHeading(1) > 'Title' + Paragraph > 'Body'")
    {
        AtomChain chain(Atom::SectionLeft);
        chain.append(Atom::SectionHeadingLeft, u"1"_s);
        chain.append(Atom::String, u"Title"_s);
        chain.append(Atom::SectionHeadingRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Body text"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::SectionRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one Section block at the top level")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Section);
            }

            THEN("The Section has two children: SectionHeading and Paragraph")
            {
                REQUIRE(blocks[0].children.size() == 2);
                REQUIRE(blocks[0].children[0].type == IR::BlockType::SectionHeading);
                REQUIRE(blocks[0].children[1].type == IR::BlockType::Paragraph);
            }

            THEN("The SectionHeading has a level attribute and title text")
            {
                const auto &heading = blocks[0].children[0];
                REQUIRE(heading.attributes["level"_L1].toInt() == 1);
                REQUIRE(heading.inlineContent.size() == 1);
                REQUIRE(heading.inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(heading.inlineContent[0].text == u"Title"_s);
            }

            THEN("The Paragraph has body text")
            {
                const auto &para = blocks[0].children[1];
                REQUIRE(para.inlineContent.size() == 1);
                REQUIRE(para.inlineContent[0].text == u"Body text"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder accumulates multiple top-level blocks",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("An atom chain with two consecutive paragraphs")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"First"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Second"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There are two Paragraph blocks")
            {
                REQUIRE(blocks.size() == 2);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[1].type == IR::BlockType::Paragraph);
            }

            THEN("Each paragraph has its own text")
            {
                REQUIRE(blocks[0].inlineContent[0].text == u"First"_s);
                REQUIRE(blocks[1].inlineContent[0].text == u"Second"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder silently skips Nop and BaseName atoms",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("An atom chain with Nop and BaseName between paragraph content")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::Nop);
        chain.append(Atom::String, u"text"_s);
        chain.append(Atom::BaseName, u"ignored"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The Nop and BaseName atoms do not affect output")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].text == u"text"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder handles stray close without crashing",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("An atom chain with ParaRight when no block is open, followed by a valid paragraph")
    {
        AtomChain chain(Atom::ParaRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"after stray close"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The stray close is ignored and the subsequent paragraph is built")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].text == u"after stray close"_s);
            }
        }
    }
}

// Note: Unclosed blocks (missing ParaRight) trigger Q_ASSERT in debug
// builds and are auto-closed in release builds. The recovery path is
// verified by code inspection, not by test, because the debug assert
// fires before the auto-close loop runs.

SCENARIO("ContentBuilder preserves valid paragraphs around block gaps",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("An atom chain with two paragraphs separated by a block-less gap")
    {
        // After the first ParaRight, no block is open. Orphan inlines
        // in that gap would be dropped (and asserted in debug builds).
        // This test verifies that the valid paragraphs on either side
        // of the gap are unaffected.
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"inside"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"after gap"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Both paragraphs are built correctly")
            {
                REQUIRE(blocks.size() == 2);
                REQUIRE(blocks[0].inlineContent[0].text == u"inside"_s);
                REQUIRE(blocks[1].inlineContent[0].text == u"after gap"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder skips FormatIf branches when no format is set",
         "[IR::ContentBuilder][IR][FormatIf]")
{
    GIVEN("An atom chain with FormatIf/FormatElse/FormatEndif around content")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"before"_s);
        chain.append(Atom::FormatIf, u"HTML"_s);
        chain.append(Atom::String, u"html-only"_s);
        chain.append(Atom::FormatElse);
        chain.append(Atom::String, u"other-only"_s);
        chain.append(Atom::FormatEndif);
        chain.append(Atom::String, u"after"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder has no format set")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Both conditional branches are skipped, only surrounding text remains")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].inlineContent.size() == 2);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[0].inlineContent[1].text == u"after"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder emits matching FormatIf branch",
         "[IR::ContentBuilder][IR][FormatIf]")
{
    GIVEN("An atom chain with FormatIf(HTML)/FormatElse/FormatEndif")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"before"_s);
        chain.append(Atom::FormatIf, u"HTML"_s);
        chain.append(Atom::String, u"html-only"_s);
        chain.append(Atom::FormatElse);
        chain.append(Atom::String, u"other-only"_s);
        chain.append(Atom::FormatEndif);
        chain.append(Atom::String, u"after"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder format is HTML")
        {
            IR::ContentBuilder builder(u"HTML"_s);
            auto blocks = builder.build(&chain.first);

            THEN("Only the if-branch text is emitted")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 3);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[0].inlineContent[1].text == u"html-only"_s);
                REQUIRE(blocks[0].inlineContent[2].text == u"after"_s);
            }
        }

        WHEN("ContentBuilder format is DocBook (non-matching)")
        {
            IR::ContentBuilder builder(u"DocBook"_s);
            auto blocks = builder.build(&chain.first);

            THEN("Only the else-branch text is emitted")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 3);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[0].inlineContent[1].text == u"other-only"_s);
                REQUIRE(blocks[0].inlineContent[2].text == u"after"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder handles FormatIf without FormatElse",
         "[IR::ContentBuilder][IR][FormatIf]")
{
    GIVEN("An atom chain with FormatIf/FormatEndif but no FormatElse")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormatIf, u"HTML"_s);
        chain.append(Atom::String, u"html-only"_s);
        chain.append(Atom::FormatEndif);
        chain.append(Atom::String, u"always"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder format matches")
        {
            IR::ContentBuilder builder(u"HTML"_s);
            auto blocks = builder.build(&chain.first);

            THEN("The conditional content and unconditional content are both emitted")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 2);
                REQUIRE(blocks[0].inlineContent[0].text == u"html-only"_s);
                REQUIRE(blocks[0].inlineContent[1].text == u"always"_s);
            }
        }

        WHEN("ContentBuilder format does not match")
        {
            IR::ContentBuilder builder(u"DocBook"_s);
            auto blocks = builder.build(&chain.first);

            THEN("Only the unconditional content is emitted")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].text == u"always"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder FormatIf comparison is case-insensitive",
         "[IR::ContentBuilder][IR][FormatIf]")
{
    GIVEN("An atom chain with FormatIf(HTML)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormatIf, u"HTML"_s);
        chain.append(Atom::String, u"conditional"_s);
        chain.append(Atom::FormatEndif);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder format is 'html' (lowercase)")
        {
            IR::ContentBuilder builder(u"html"_s);
            auto blocks = builder.build(&chain.first);

            THEN("The conditional content is emitted (case-insensitive match)")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].text == u"conditional"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder ignores stray FormatEndif at top level",
         "[IR::ContentBuilder][IR][FormatIf]")
{
    GIVEN("An atom chain with a stray FormatEndif between two paragraphs")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"before"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::FormatEndif);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"after"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Both paragraphs are produced; the stray FormatEndif is ignored")
            {
                REQUIRE(blocks.size() == 2);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[1].inlineContent[0].text == u"after"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder ignores stray FormatElse at top level",
         "[IR::ContentBuilder][IR][FormatIf]")
{
    GIVEN("An atom chain with a stray FormatElse between two paragraphs")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"before"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::FormatElse);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"after"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Both paragraphs are produced; the stray FormatElse is ignored")
            {
                REQUIRE(blocks.size() == 2);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[1].inlineContent[0].text == u"after"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder skips BriefLeft/BriefRight content in body",
         "[IR::ContentBuilder][IR][Brief]")
{
    GIVEN("An atom chain with brief content followed by a regular paragraph")
    {
        AtomChain chain(Atom::BriefLeft);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Brief text."_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::BriefRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Body text."_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The brief content is excluded; only the body paragraph appears")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].inlineContent[0].text == u"Body text."_s);
            }
        }
    }
}

SCENARIO("ContentBuilder excludes sections and paragraphs inside brief",
         "[IR::ContentBuilder][IR][Brief]")
{
    GIVEN("An atom chain with a section and paragraph inside BriefLeft/BriefRight")
    {
        AtomChain chain(Atom::BriefLeft);
        chain.append(Atom::SectionLeft);
        chain.append(Atom::SectionHeadingLeft, u"1"_s);
        chain.append(Atom::String, u"Brief heading"_s);
        chain.append(Atom::SectionHeadingRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"brief para"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::SectionRight);
        chain.append(Atom::BriefRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Body text."_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("No blocks from brief content appear; only the body paragraph")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].inlineContent[0].text == u"Body text."_s);
            }
        }
    }
}
