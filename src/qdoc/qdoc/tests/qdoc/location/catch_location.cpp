// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <QtCore/qregularexpression.h>

#include <qdoc/config.h>
#include <qdoc/location.h>

#include <cstdlib>

using namespace Qt::Literals::StringLiterals;

namespace {

// Location keeps its diagnostic state in statics, so each scenario starts
// from a known configuration. The warning limit makes the internal warning
// count observable through Location::exitCode(): an exceeded limit returns
// the count itself, a non-exceeded limit returns the error state.
void setWarningLimit(const char *limit)
{
    Config::instance().setStringList(QStringLiteral("warninglimit"),
                                     QStringList(limit));
    Config::instance().setStringList(QStringLiteral("warninglimit.enabled"),
                                     { u"true"_s });
}

} // namespace

SCENARIO("Identical warnings are reported once per project run",
         "[location][warning-policy]")
{
    setWarningLimit("1");
    Location::initialize();

    Location loc(u"/tmp/source.qdoc"_s);
    loc.setLineNo(12);
    loc.warning(u"Can't link to 'Target'"_s);
    loc.warning(u"Can't link to 'Target'"_s);
    loc.warning(u"Can't link to 'Target'"_s);

    // The three encounters are one warning: the count does not exceed the
    // limit and no error is recorded.
    REQUIRE(Location::exitCode() == EXIT_SUCCESS);

    Location::terminate();
}

SCENARIO("Warnings at different lines report independently",
         "[location][warning-policy]")
{
    setWarningLimit("1");
    Location::initialize();

    Location loc(u"/tmp/source.qdoc"_s);
    loc.setLineNo(12);
    loc.warning(u"Can't link to 'Target'"_s);
    loc.setLineNo(18);
    loc.warning(u"Can't link to 'Target'"_s);

    // Two lines are two diagnostics, even when the message matches.
    REQUIRE(Location::exitCode() == 2);

    Location::terminate();
}

SCENARIO("Warnings with different details report independently",
         "[location][warning-policy]")
{
    setWarningLimit("1");
    Location::initialize();

    Location loc(u"/tmp/source.qdoc"_s);
    loc.setLineNo(12);
    loc.warning(u"Can't link to 'Target'"_s, u"first details"_s);
    loc.warning(u"Can't link to 'Target'"_s, u"second details"_s);

    REQUIRE(Location::exitCode() == 2);

    Location::terminate();
}

SCENARIO("The include stack is part of the warning identity",
         "[location][warning-policy]")
{
    setWarningLimit("1");
    Location::initialize();

    Location a(u"/tmp/page.qdoc"_s);
    a.setLineNo(5);
    a.push(u"/tmp/inc.qdoc"_s);
    a.setLineNo(3);

    Location b = a;
    b.warning(u"Can't link to 'Target'"_s);
    a.warning(u"Can't link to 'Target'"_s); // same stack: suppressed

    Location c(u"/tmp/page.qdoc"_s);
    c.setLineNo(5);
    c.push(u"/tmp/other.qdoc"_s);
    c.setLineNo(3);
    c.warning(u"Can't link to 'Target'"_s); // different stack: reported

    REQUIRE(Location::exitCode() == 2);

    Location::terminate();
}

SCENARIO("The warning set resets between project runs",
         "[location][warning-policy]")
{
    Config::instance().setStringList(QStringLiteral("project"), { u"P"_s });
    setWarningLimit("0");

    Location::initialize();
    Location loc(u"/tmp/source.qdoc"_s);
    loc.setLineNo(12);
    loc.warning(u"Can't link to 'Target'"_s);
    REQUIRE(Location::exitCode() == 1);
    Location::terminate();

    // A new project run with the identical signature reports it again.
    Location::initialize();
    loc.warning(u"Can't link to 'Target'"_s);
    REQUIRE(Location::exitCode() == 1);
    Location::terminate();
}

SCENARIO("Warnings from different projects report independently",
         "[location][warning-policy]")
{
    setWarningLimit("0");
    Location loc(u"/tmp/source.qdoc"_s);
    loc.setLineNo(12);

    Config::instance().setStringList(QStringLiteral("project"), { u"First"_s });
    Location::initialize();
    loc.warning(u"Can't link to 'Target'"_s);
    REQUIRE(Location::exitCode() == 1);
    Location::terminate();

    Config::instance().setStringList(QStringLiteral("project"), { u"Second"_s });
    Location::initialize();
    loc.warning(u"Can't link to 'Target'"_s);
    REQUIRE(Location::exitCode() == 1);
    Location::terminate();
}
