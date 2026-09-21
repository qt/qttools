// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch_conversions/qdoc_catch_conversions.h>

#include <catch/catch.hpp>

#include <qdoc/atom.h>
#include <qdoc/genustypes.h>
#include <qdoc/text.h>

QT_USE_NAMESPACE

namespace {

Atom *firstLinkAtom(Text &text)
{
    for (Atom *atom = text.firstAtom(); atom; atom = atom->next())
        if (atom->isLinkAtom())
            return atom;
    return nullptr;
}

} // namespace

SCENARIO("Appending an atom to a Text preserves its dynamic type", "[Text][Atom]")
{
    GIVEN("A LinkAtom carrying square-bracket metadata") {
        LinkAtom link("Widget", "QML");
        REQUIRE(link.genus() == Genus::QML);

        WHEN("It is appended through a base-class reference") {
            const Atom &base = link;
            Text text;
            text << base;

            THEN("The stored copy is still a LinkAtom") {
                REQUIRE(text.firstAtom() != nullptr);
                REQUIRE(text.firstAtom()->isLinkAtom());
            }

            THEN("The copy is a distinct object with the metadata intact") {
                CHECK(text.firstAtom() != &link);
                CHECK(text.firstAtom()->genus() == Genus::QML);
                CHECK(text.firstAtom()->type() == Atom::Link);
                CHECK(text.firstAtom()->string() == QStringLiteral("Widget"));
            }
        }

        WHEN("It is appended through a base-class reference to a non-empty Text") {
            Text text;
            text << Atom(Atom::String, "see ");
            text << static_cast<const Atom &>(link);

            THEN("The appended atom is a LinkAtom with the metadata intact") {
                Atom *appended = text.firstAtom()->next();
                REQUIRE(appended != nullptr);
                REQUIRE(appended->isLinkAtom());
                CHECK(appended->genus() == Genus::QML);
            }
        }
    }
}

SCENARIO("Copying a Text preserves LinkAtom metadata", "[Text][Atom]")
{
    GIVEN("A Text containing a qualified link between plain atoms") {
        Text original;
        original << Atom(Atom::String, "see ") << LinkAtom("Widget", "QML")
                 << Atom(Atom::String, " for details");
        REQUIRE(firstLinkAtom(original) != nullptr);

        WHEN("The Text is copy-constructed") {
            Text copy(original);

            THEN("The copy holds its own LinkAtom with the metadata intact") {
                Atom *copied = firstLinkAtom(copy);
                REQUIRE(copied != nullptr);
                CHECK(copied != firstLinkAtom(original));
                CHECK(copied->genus() == Genus::QML);
                CHECK(copied->string() == QStringLiteral("Widget"));
            }
        }

        WHEN("The Text is copy-assigned over existing content") {
            Text copy;
            copy << Atom(Atom::String, "to be replaced");
            copy = original;

            THEN("The assigned-to Text holds a LinkAtom with the metadata intact") {
                Atom *copied = firstLinkAtom(copy);
                REQUIRE(copied != nullptr);
                CHECK(copied->genus() == Genus::QML);
            }
        }

        WHEN("The Text is appended to another Text") {
            Text target;
            target << Atom(Atom::ParaLeft);
            target << original;

            THEN("The appended range holds a LinkAtom with the metadata intact") {
                Atom *copied = firstLinkAtom(target);
                REQUIRE(copied != nullptr);
                CHECK(copied->genus() == Genus::QML);
            }
        }

        WHEN("A subrange containing the link is extracted") {
            Text sub = Text::subText(original.firstAtom(), nullptr);

            THEN("The extracted range holds a LinkAtom with the metadata intact") {
                Atom *copied = firstLinkAtom(sub);
                REQUIRE(copied != nullptr);
                CHECK(copied->genus() == Genus::QML);
            }
        }
    }
}

/*
 * The scenarios below characterize the copy behavior Text::operator<< has
 * today, including its two-string cap and empty-second-string drop. They
 * pin the pre-clone() semantics so that introducing a virtual clone()
 * (Core Guidelines C.130) is provably behavior-preserving; a deliberate
 * change to how many strings survive a copy must then show up as an
 * explicit diff to these assertions instead of riding along unnoticed.
 */
SCENARIO("Characterization: copying atoms into a Text caps the surviving strings",
         "[Text][Atom][characterization]")
{
    GIVEN("An atom constructed without a string") {
        Atom bare(Atom::ParaLeft);
        REQUIRE(bare.count() == 1);
        REQUIRE(bare.string().isEmpty());

        WHEN("It is appended to a Text") {
            Text text;
            text << bare;

            THEN("The copy keeps the single empty string entry") {
                REQUIRE(text.firstAtom() != nullptr);
                CHECK(text.firstAtom()->type() == Atom::ParaLeft);
                REQUIRE(text.firstAtom()->count() == 1);
                CHECK(text.firstAtom()->string().isEmpty());
            }
        }
    }

    GIVEN("A one-string atom") {
        Atom plain(Atom::String, "plain");

        WHEN("It is appended to a Text") {
            Text text;
            text << plain;

            THEN("Type and string are preserved and it is not a LinkAtom") {
                REQUIRE(text.firstAtom() != nullptr);
                CHECK_FALSE(text.firstAtom()->isLinkAtom());
                CHECK(text.firstAtom()->type() == Atom::String);
                REQUIRE(text.firstAtom()->count() == 1);
                CHECK(text.firstAtom()->string() == QStringLiteral("plain"));
                CHECK(text.firstAtom()->genus() == Genus::DontCare);
            }
        }
    }

    GIVEN("A two-string atom") {
        Atom tagged(Atom::Target, "first", "second");
        REQUIRE(tagged.count() == 2);

        WHEN("It is appended to a Text") {
            Text text;
            text << tagged;

            THEN("Both strings survive the copy") {
                REQUIRE(text.firstAtom() != nullptr);
                REQUIRE(text.firstAtom()->count() == 2);
                CHECK(text.firstAtom()->string() == QStringLiteral("first"));
                CHECK(text.firstAtom()->string(1) == QStringLiteral("second"));
            }
        }
    }

    GIVEN("A two-string atom whose second string is empty") {
        Atom tagged(Atom::Target, "first");
        tagged.append(QString());
        REQUIRE(tagged.count() == 2);

        WHEN("It is appended to a Text") {
            Text text;
            text << tagged;

            THEN("The empty second string is dropped by the copy") {
                REQUIRE(text.firstAtom() != nullptr);
                REQUIRE(text.firstAtom()->count() == 1);
                CHECK(text.firstAtom()->string() == QStringLiteral("first"));
            }
        }
    }

    GIVEN("A three-string atom") {
        Atom tagged(Atom::Target, "first", "second");
        tagged.append("third");
        REQUIRE(tagged.count() == 3);

        WHEN("It is appended to a Text") {
            Text text;
            text << tagged;

            THEN("Only the first two strings survive the copy") {
                REQUIRE(text.firstAtom() != nullptr);
                REQUIRE(text.firstAtom()->count() == 2);
                CHECK(text.firstAtom()->string() == QStringLiteral("first"));
                CHECK(text.firstAtom()->string(1) == QStringLiteral("second"));
            }
        }
    }

    GIVEN("A LinkAtom with an extra appended string") {
        LinkAtom link("Widget", "QML");
        link.append("extra");
        REQUIRE(link.count() == 2);

        WHEN("It is appended through a base-class reference") {
            Text text;
            text << static_cast<const Atom &>(link);

            THEN("The metadata survives but only the first string does") {
                REQUIRE(text.firstAtom() != nullptr);
                REQUIRE(text.firstAtom()->isLinkAtom());
                CHECK(text.firstAtom()->genus() == Genus::QML);
                REQUIRE(text.firstAtom()->count() == 1);
                CHECK(text.firstAtom()->string() == QStringLiteral("Widget"));
            }
        }
    }
}

