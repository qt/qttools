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

SCENARIO("ContentBuilder excludes block-level atoms inside brief",
         "[IR::ContentBuilder][IR][Brief]")
{
    GIVEN("An atom chain with a code block and list inside BriefLeft/BriefRight")
    {
        AtomChain chain(Atom::BriefLeft);
        chain.append(Atom::Code, u"int x = 0;"_s);
        chain.append(Atom::ListLeft, u"bullet"_s);
        chain.append(Atom::ListItemLeft);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"item"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::ListItemRight);
        chain.append(Atom::ListRight);
        chain.append(Atom::HR);
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

SCENARIO("ContentBuilder brief suppression prevents state corruption from unmatched atoms",
         "[IR::ContentBuilder][IR][Brief]")
{
    GIVEN("An atom chain with unmatched open atoms inside BriefLeft/BriefRight")
    {
        // ListLeft without ListRight and SectionLeft without SectionRight
        // inside brief. If brief suppression failed to prevent these from
        // reaching dispatch, the block path would be corrupted and the
        // subsequent paragraph would nest incorrectly or trigger asserts.
        AtomChain chain(Atom::BriefLeft);
        chain.append(Atom::ListLeft, u"bullet"_s);
        chain.append(Atom::SectionLeft);
        chain.append(Atom::BriefRight);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"after brief"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The builder state is clean; only the body paragraph appears")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].text == u"after brief"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces Link from AutoLink atom",
         "[IR::ContentBuilder][IR][Link]")
{
    GIVEN("An atom chain with AutoLink inside a paragraph")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::AutoLink, u"QString"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Link inline with href and text child")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                const auto &link = blocks[0].inlineContent[0];
                REQUIRE(link.type == IR::InlineType::Link);
                REQUIRE(link.href == u"QString"_s);
                REQUIRE(link.children.size() == 1);
                REQUIRE(link.children[0].type == IR::InlineType::Text);
                REQUIRE(link.children[0].text == u"QString"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces Link from NavAutoLink atom",
         "[IR::ContentBuilder][IR][Link]")
{
    GIVEN("An atom chain with NavAutoLink inside a paragraph")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::NavAutoLink, u"QWidget"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Link inline")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                const auto &link = blocks[0].inlineContent[0];
                REQUIRE(link.type == IR::InlineType::Link);
                REQUIRE(link.href == u"QWidget"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces Link from Link atom with link formatting",
         "[IR::ContentBuilder][IR][Link]")
{
    GIVEN("An atom chain: Link -> FormattingLeft(link) -> String -> FormattingRight(link)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::Link, u"qstring.html"_s);
        chain.append(Atom::FormattingLeft, u"link"_s);
        chain.append(Atom::String, u"QString class"_s);
        chain.append(Atom::FormattingRight, u"link"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Link inline with the display text as child")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                const auto &link = blocks[0].inlineContent[0];
                REQUIRE(link.type == IR::InlineType::Link);
                REQUIRE(link.href == u"qstring.html"_s);
                REQUIRE(link.children.size() == 1);
                REQUIRE(link.children[0].type == IR::InlineType::Text);
                REQUIRE(link.children[0].text == u"QString class"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder handles Link atom followed by non-link formatting",
         "[IR::ContentBuilder][IR][Link]")
{
    GIVEN("A Link atom followed by FormattingLeft(link), bold text inside, FormattingRight(link)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::Link, u"target.html"_s);
        chain.append(Atom::FormattingLeft, u"link"_s);
        chain.append(Atom::FormattingLeft, u"bold"_s);
        chain.append(Atom::String, u"bold link"_s);
        chain.append(Atom::FormattingRight, u"bold"_s);
        chain.append(Atom::FormattingRight, u"link"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The link contains a bold child with text")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                const auto &link = blocks[0].inlineContent[0];
                REQUIRE(link.type == IR::InlineType::Link);
                REQUIRE(link.href == u"target.html"_s);
                REQUIRE(link.children.size() == 1);
                REQUIRE(link.children[0].type == IR::InlineType::Bold);
                REQUIRE(link.children[0].children[0].text == u"bold link"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces Code inline from C atom",
         "[IR::ContentBuilder][IR][Inline]")
{
    GIVEN("An atom chain with a C atom inside a paragraph")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"See "_s);
        chain.append(Atom::C, u"QString"_s);
        chain.append(Atom::String, u" for details."_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has three inlines: Text, Code, Text")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 3);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[0].text == u"See "_s);
                REQUIRE(blocks[0].inlineContent[1].type == IR::InlineType::Code);
                REQUIRE(blocks[0].inlineContent[1].text == u"QString"_s);
                REQUIRE(blocks[0].inlineContent[2].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[2].text == u" for details."_s);
            }
        }
    }
}

SCENARIO("ContentBuilder wraps bold formatting into Bold inline container",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with FormattingLeft(bold) / String / FormattingRight(bold)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormattingLeft, u"bold"_s);
        chain.append(Atom::String, u"important"_s);
        chain.append(Atom::FormattingRight, u"bold"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Bold inline container with text child")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Bold);
                REQUIRE(blocks[0].inlineContent[0].children.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].children[0].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[0].children[0].text == u"important"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder wraps italic formatting into Italic inline container",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with FormattingLeft(italic) / String / FormattingRight(italic)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormattingLeft, u"italic"_s);
        chain.append(Atom::String, u"emphasis"_s);
        chain.append(Atom::FormattingRight, u"italic"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Italic inline container")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Italic);
                REQUIRE(blocks[0].inlineContent[0].children[0].text == u"emphasis"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder maps teletype formatting to Teletype inline",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with FormattingLeft(teletype) / String / FormattingRight(teletype)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormattingLeft, u"teletype"_s);
        chain.append(Atom::String, u"monospace"_s);
        chain.append(Atom::FormattingRight, u"teletype"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Teletype inline container")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Teletype);
                REQUIRE(blocks[0].inlineContent[0].children[0].text == u"monospace"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder maps uicontrol formatting to Bold inline",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with FormattingLeft(uicontrol) / String / FormattingRight(uicontrol)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormattingLeft, u"uicontrol"_s);
        chain.append(Atom::String, u"File > Save"_s);
        chain.append(Atom::FormattingRight, u"uicontrol"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has one Bold inline (uicontrol maps to Bold)")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Bold);
                REQUIRE(blocks[0].inlineContent[0].children[0].text == u"File > Save"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder skips index, trademark, notranslate, and span formatting",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with index formatting around text in a paragraph")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"before"_s);
        chain.append(Atom::FormattingLeft, u"index"_s);
        chain.append(Atom::String, u"indexed"_s);
        chain.append(Atom::FormattingRight, u"index"_s);
        chain.append(Atom::String, u"after"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The index formatting is transparent; text appears as plain inlines")
            {
                REQUIRE(blocks[0].inlineContent.size() == 3);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[0].inlineContent[1].text == u"indexed"_s);
                REQUIRE(blocks[0].inlineContent[2].text == u"after"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder maps subscript and superscript formatting",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with subscript and superscript formatting")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"H"_s);
        chain.append(Atom::FormattingLeft, u"subscript"_s);
        chain.append(Atom::String, u"2"_s);
        chain.append(Atom::FormattingRight, u"subscript"_s);
        chain.append(Atom::String, u"O"_s);
        chain.append(Atom::FormattingLeft, u"superscript"_s);
        chain.append(Atom::String, u"+"_s);
        chain.append(Atom::FormattingRight, u"superscript"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Subscript and superscript are properly mapped")
            {
                REQUIRE(blocks[0].inlineContent.size() == 4);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[1].type == IR::InlineType::Subscript);
                REQUIRE(blocks[0].inlineContent[1].children[0].text == u"2"_s);
                REQUIRE(blocks[0].inlineContent[2].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[3].type == IR::InlineType::Superscript);
                REQUIRE(blocks[0].inlineContent[3].children[0].text == u"+"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder maps parameter formatting to Parameter inline",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("An atom chain with parameter formatting")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::FormattingLeft, u"parameter"_s);
        chain.append(Atom::String, u"arg"_s);
        chain.append(Atom::FormattingRight, u"parameter"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The parameter formatting produces a Parameter inline")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Parameter);
                REQUIRE(blocks[0].inlineContent[0].children[0].text == u"arg"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces LineBreak inline from BR atom",
         "[IR::ContentBuilder][IR][Misc]")
{
    GIVEN("An atom chain with BR between text in a paragraph")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"line one"_s);
        chain.append(Atom::BR);
        chain.append(Atom::String, u"line two"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("The paragraph has Text, LineBreak, Text inlines")
            {
                REQUIRE(blocks[0].inlineContent.size() == 3);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[0].text == u"line one"_s);
                REQUIRE(blocks[0].inlineContent[1].type == IR::InlineType::LineBreak);
                REQUIRE(blocks[0].inlineContent[2].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[2].text == u"line two"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces CodeBlock from Code atom",
         "[IR::ContentBuilder][IR][CodeBlock]")
{
    GIVEN("An atom chain with a Code atom")
    {
        AtomChain chain(Atom::Code, u"int x = 42;"_s, u"cpp"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one CodeBlock with language attribute")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::CodeBlock);
                REQUIRE(blocks[0].attributes["language"_L1].toString() == u"cpp"_s);
            }

            THEN("The code content is the inline text")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].type == IR::InlineType::Text);
                REQUIRE(blocks[0].inlineContent[0].text == u"int x = 42;"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces CodeBlock from CodeBad atom with bad attribute",
         "[IR::ContentBuilder][IR][CodeBlock]")
{
    GIVEN("An atom chain with a CodeBad atom")
    {
        AtomChain chain(Atom::CodeBad, u"broken code"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one CodeBlock with language=cpp and bad=true")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::CodeBlock);
                REQUIRE(blocks[0].attributes["language"_L1].toString() == u"cpp"_s);
                REQUIRE(blocks[0].attributes["bad"_L1].toBool() == true);
            }
        }
    }
}

SCENARIO("ContentBuilder produces CodeBlock from Qml atom",
         "[IR::ContentBuilder][IR][CodeBlock]")
{
    GIVEN("An atom chain with a Qml atom")
    {
        AtomChain chain(Atom::Qml, u"Item { width: 100 }"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one CodeBlock with language=qml")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::CodeBlock);
                REQUIRE(blocks[0].attributes["language"_L1].toString() == u"qml"_s);
            }

            THEN("The code content is the inline text")
            {
                REQUIRE(blocks[0].inlineContent.size() == 1);
                REQUIRE(blocks[0].inlineContent[0].text == u"Item { width: 100 }"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder defaults Code language to cpp when no language string",
         "[IR::ContentBuilder][IR][CodeBlock]")
{
    GIVEN("A Code atom with only one string parameter (no language)")
    {
        AtomChain chain(Atom::Code, u"int y = 0;"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Language defaults to cpp")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].attributes["language"_L1].toString() == u"cpp"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder builds a bullet list with items",
         "[IR::ContentBuilder][IR][List]")
{
    GIVEN("An atom chain representing a bullet list with two items")
    {
        AtomChain chain(Atom::ListLeft, u"bullet"_s);
        chain.append(Atom::ListItemLeft);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"First item"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::ListItemRight);
        chain.append(Atom::ListItemLeft);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Second item"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::ListItemRight);
        chain.append(Atom::ListRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one List block with listType attribute")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::List);
                REQUIRE(blocks[0].attributes["listType"_L1].toString() == u"bullet"_s);
            }

            THEN("The list has two ListItem children")
            {
                REQUIRE(blocks[0].children.size() == 2);
                REQUIRE(blocks[0].children[0].type == IR::BlockType::ListItem);
                REQUIRE(blocks[0].children[1].type == IR::BlockType::ListItem);
            }

            THEN("Each ListItem contains a Paragraph with text")
            {
                const auto &item1 = blocks[0].children[0];
                REQUIRE(item1.children.size() == 1);
                REQUIRE(item1.children[0].type == IR::BlockType::Paragraph);
                REQUIRE(item1.children[0].inlineContent[0].text == u"First item"_s);

                const auto &item2 = blocks[0].children[1];
                REQUIRE(item2.children[0].inlineContent[0].text == u"Second item"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder builds a Note block",
         "[IR::ContentBuilder][IR][Admonition]")
{
    GIVEN("An atom chain with NoteLeft/NoteRight wrapping a paragraph")
    {
        AtomChain chain(Atom::NoteLeft);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"This is important."_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::NoteRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one Note block containing a Paragraph")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Note);
                REQUIRE(blocks[0].children.size() == 1);
                REQUIRE(blocks[0].children[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].children[0].inlineContent[0].text
                        == u"This is important."_s);
            }
        }
    }
}

SCENARIO("ContentBuilder builds a Warning block",
         "[IR::ContentBuilder][IR][Admonition]")
{
    GIVEN("An atom chain with WarningLeft/WarningRight wrapping a paragraph")
    {
        AtomChain chain(Atom::WarningLeft);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"Dangerous operation."_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::WarningRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one Warning block containing a Paragraph")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Warning);
                REQUIRE(blocks[0].children.size() == 1);
                REQUIRE(blocks[0].children[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[0].children[0].inlineContent[0].text
                        == u"Dangerous operation."_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces HorizontalRule from HR atom",
         "[IR::ContentBuilder][IR][Misc]")
{
    GIVEN("An atom chain with HR between two paragraphs")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"above"_s);
        chain.append(Atom::ParaRight);
        chain.append(Atom::HR);
        chain.append(Atom::ParaLeft);
        chain.append(Atom::String, u"below"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There are three blocks: Paragraph, HorizontalRule, Paragraph")
            {
                REQUIRE(blocks.size() == 3);
                REQUIRE(blocks[0].type == IR::BlockType::Paragraph);
                REQUIRE(blocks[1].type == IR::BlockType::HorizontalRule);
                REQUIRE(blocks[2].type == IR::BlockType::Paragraph);
            }
        }
    }
}

SCENARIO("ContentBuilder produces Div for AnnotatedList atom",
         "[IR::ContentBuilder][IR][Misc]")
{
    GIVEN("An atom chain with an AnnotatedList atom")
    {
        AtomChain chain(Atom::AnnotatedList, u"mygroup"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one Div block with annotatedList attribute")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Div);
                REQUIRE(blocks[0].attributes["annotatedList"_L1].toString()
                        == u"mygroup"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder produces Div for GeneratedList atom",
         "[IR::ContentBuilder][IR][Misc]")
{
    GIVEN("An atom chain with a GeneratedList atom")
    {
        AtomChain chain(Atom::GeneratedList, u"classes"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("There is one Div block with generatedList attribute")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].type == IR::BlockType::Div);
                REQUIRE(blocks[0].attributes["generatedList"_L1].toString()
                        == u"classes"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder can be reused for multiple build() calls",
         "[IR::ContentBuilder][IR]")
{
    GIVEN("A ContentBuilder used to build two different chains")
    {
        IR::ContentBuilder builder;

        AtomChain chain1(Atom::ParaLeft);
        chain1.append(Atom::String, u"first"_s);
        chain1.append(Atom::ParaRight);

        AtomChain chain2(Atom::ParaLeft);
        chain2.append(Atom::String, u"second"_s);
        chain2.append(Atom::ParaRight);

        WHEN("build() is called twice")
        {
            auto blocks1 = builder.build(&chain1.first);
            auto blocks2 = builder.build(&chain2.first);

            THEN("Each result is independent")
            {
                REQUIRE(blocks1.size() == 1);
                REQUIRE(blocks1[0].inlineContent[0].text == u"first"_s);
                REQUIRE(blocks2.size() == 1);
                REQUIRE(blocks2[0].inlineContent[0].text == u"second"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder handles stray FormattingRight without matching left",
         "[IR::ContentBuilder][IR][Formatting]")
{
    GIVEN("A paragraph with FormattingRight(bold) but no FormattingLeft(bold)")
    {
        AtomChain chain(Atom::ParaLeft);
        chain.append(Atom::String, u"before"_s);
        chain.append(Atom::FormattingRight, u"bold"_s);
        chain.append(Atom::String, u"after"_s);
        chain.append(Atom::ParaRight);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("Both text strings appear as plain inlines")
            {
                REQUIRE(blocks.size() == 1);
                REQUIRE(blocks[0].inlineContent.size() == 2);
                REQUIRE(blocks[0].inlineContent[0].text == u"before"_s);
                REQUIRE(blocks[0].inlineContent[1].text == u"after"_s);
            }
        }
    }
}

SCENARIO("ContentBuilder drops Link atom outside any block context",
         "[IR::ContentBuilder][IR][Link]")
{
    GIVEN("A Link atom with no enclosing paragraph or section")
    {
        AtomChain chain(Atom::Link, u"foo.html"_s);

        WHEN("ContentBuilder processes the chain")
        {
            IR::ContentBuilder builder;
            auto blocks = builder.build(&chain.first);

            THEN("No blocks are produced and m_inLink is not set")
            {
                REQUIRE(blocks.isEmpty());
            }
        }
    }
}
