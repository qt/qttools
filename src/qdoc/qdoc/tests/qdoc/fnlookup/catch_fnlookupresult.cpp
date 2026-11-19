// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qdoc/fnlookupresult.h>

#include <QtCore/qlist.h>

#include <catch/catch.hpp>

QT_BEGIN_NAMESPACE

/*
  Focused coverage for the \fn lookup outcome: the three-way
  FnLookupResult and the candidate reduction it is built from. The
  reduction is what the hidden-friend-in-class-template fallback applies
  to its matching candidates, so the outcome/uniqueness rules are
  exercised here at the helper level, with owned opaque nodes:
  a repeated identical node remains one match, two distinct matching
  nodes yield ambiguity, and no candidate yields no match.
 */

namespace {

struct TestNode { };
using TestLookupResult = FnLookupResultFor<TestNode>;

} // namespace

TEST_CASE("FnLookupResult reports three mutually exclusive states", "[qdoc][fnlookup]")
{
    TestNode fn;

    TestLookupResult result;
    REQUIRE(result.status() == TestLookupResult::Status::NoMatch);
    REQUIRE_FALSE(result.hasMatch());
    REQUIRE_FALSE(result.isAmbiguous());
    CHECK(result.node() == nullptr);

    result.setMatched(&fn);
    REQUIRE(result.status() == TestLookupResult::Status::Matched);
    REQUIRE(result.hasMatch());
    REQUIRE_FALSE(result.isAmbiguous());
    CHECK(result.node() == &fn);

    result.setAmbiguous();
    REQUIRE(result.status() == TestLookupResult::Status::Ambiguous);
    REQUIRE_FALSE(result.hasMatch());
    REQUIRE(result.isAmbiguous());
    CHECK(result.node() == nullptr);

    result.reset();
    REQUIRE(result.status() == TestLookupResult::Status::NoMatch);
    REQUIRE_FALSE(result.hasMatch());
    REQUIRE(result.node() == nullptr);
}

TEST_CASE("resolveFnLookupMatches deduplicates a repeated node into one match",
          "[qdoc][fnlookup]")
{
    TestNode fn;
    const QList<TestNode *> candidates{ &fn, &fn, &fn };

    const TestLookupResult result = resolveFnLookupMatches(candidates);

    REQUIRE(result.status() == TestLookupResult::Status::Matched);
    CHECK(result.node() == &fn);
}

TEST_CASE("resolveFnLookupMatches reports distinct matching nodes as ambiguous",
          "[qdoc][fnlookup]")
{
    TestNode first;
    TestNode second;
    const QList<TestNode *> candidates{ &first, &second };

    const TestLookupResult result = resolveFnLookupMatches(candidates);

    REQUIRE(result.status() == TestLookupResult::Status::Ambiguous);
    CHECK(result.node() == nullptr);
}

TEST_CASE("resolveFnLookupMatches reports an empty candidate set as no match",
          "[qdoc][fnlookup]")
{
    const QList<TestNode *> candidates;

    const TestLookupResult result = resolveFnLookupMatches(candidates);

    REQUIRE(result.status() == TestLookupResult::Status::NoMatch);
    CHECK(result.node() == nullptr);
}

TEST_CASE("existing spelling takes precedence over renamed templates", "[qdoc][fnlookup]")
{
    TestNode ordinary;
    TestNode renamed;
    const bool renamedFirst = GENERATE(false, true);
    const QList<TestNode *> candidates = renamedFirst
            ? QList<TestNode *>{ &renamed, &ordinary } : QList<TestNode *>{ &ordinary, &renamed };
    int renamedCalls = 0;
    int fallbackCalls = 0;

    const auto result = resolveFnLookupCandidates(candidates, [&](TestNode *node) {
        return node == &ordinary;
    }, [&](TestNode *) {
        ++renamedCalls;
        return true;
    }, [&](TestNode *) {
        ++fallbackCalls;
        return true;
    });

    CHECK(result.node() == &ordinary);
    CHECK(renamedCalls == 0);
    CHECK(fallbackCalls == 0);
}

TEST_CASE("renamed ordinary templates precede hidden friends regardless of candidate order",
          "[qdoc][fnlookup]")
{
    TestNode ordinary;
    TestNode fallback;
    const bool fallbackFirst = GENERATE(false, true);
    const QList<TestNode *> candidates = fallbackFirst
            ? QList<TestNode *>{ &fallback, &ordinary } : QList<TestNode *>{ &ordinary, &fallback };
    int fallbackCalls = 0;

    const auto result = resolveFnLookupCandidates(candidates, [](TestNode *) { return false; },
                                                [&](TestNode *node) {
        return node == &ordinary;
    }, [&](TestNode *) {
        ++fallbackCalls;
        return true;
    });

    CHECK(result.node() == &ordinary);
    CHECK(fallbackCalls == 0);
}

TEST_CASE("candidate lookup retains ordinary first-match behavior", "[qdoc][fnlookup]")
{
    TestNode first;
    TestNode second;
    const QList<TestNode *> candidates{ &first, &second };
    const auto result = resolveFnLookupCandidates(candidates, [](TestNode *) { return true; },
                                                [](TestNode *) { return true; },
                                                [](TestNode *) { return false; });
    CHECK(result.node() == &first);
}

TEST_CASE("renamed ordinary lookup retains first-match behavior", "[qdoc][fnlookup]")
{
    TestNode first;
    TestNode second;
    const QList<TestNode *> candidates{ &first, &second };
    const auto result = resolveFnLookupCandidates(candidates, [](TestNode *) { return false; },
                                                [](TestNode *) { return true; },
                                                [](TestNode *) { return false; });
    CHECK(result.node() == &first);
}

TEST_CASE("fallback lookup requires a unique distinct node", "[qdoc][fnlookup]")
{
    TestNode first;
    TestNode second;
    const QList<TestNode *> candidates{ &first, &first, &second };
    const int matchCount = GENERATE(0, 1, 2);
    const auto result = resolveFnLookupCandidates(candidates, [](TestNode *) { return false; },
                                                [](TestNode *) { return false; },
                                                [&](TestNode *node) {
        return matchCount == 2 || (matchCount == 1 && node == &first);
    });

    CHECK(result.hasMatch() == (matchCount == 1));
    CHECK(result.isAmbiguous() == (matchCount == 2));
    CHECK(result.node() == (matchCount == 1 ? &first : nullptr));
}

QT_END_NAMESPACE
