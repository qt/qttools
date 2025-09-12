// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/inclusionflags.h>
#include <qdoc/inclusionpolicy.h>
#include <qdoc/nodecontext.h>
#include <qdoc/inclusionfilter.h>

TEST_CASE("NodeContext toFlags() behavior", "[NodeContext]")
{
    SECTION("Private function node returns correct flags")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateFunction) == InclusionFlag::PrivateFunction);
    }

    SECTION("Private class node returns correct flags")
    {
        NodeContext context;
        context.type = NodeType::Class;
        context.isPrivate = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateType) == InclusionFlag::PrivateType);
    }

    SECTION("Private variable node returns correct flags")
    {
        NodeContext context;
        context.type = NodeType::Variable;
        context.isPrivate = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateVariable) == InclusionFlag::PrivateVariable);
    }
}

TEST_CASE("InclusionPolicy basic functionality", "[InclusionPolicy]")
{
    SECTION("Default InclusionPolicy")
    {
        InclusionPolicy policy;
        REQUIRE(policy.includePrivate == false);
        REQUIRE(policy.includePrivateFunction == false);
        REQUIRE(policy.includePrivateType == false);
        REQUIRE(policy.includePrivateVariable == false);
    }

    SECTION("InclusionPolicy with custom values")
    {
        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.includePrivateFunction = true;
        policy.includePrivateType = true;
        policy.includePrivateVariable = true;

        REQUIRE(policy.includePrivate == true);
        REQUIRE(policy.includePrivateFunction == true);
        REQUIRE(policy.includePrivateType == true);
        REQUIRE(policy.includePrivateVariable == true);
    }
}

TEST_CASE("InclusionFilter basic functionality", "[InclusionFilter]")
{
    SECTION("Public node is included by default")
    {
        NodeContext context;
        context.isPrivate = false;

        InclusionPolicy policy;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == true);
    }

    SECTION("Private node is excluded by default")
    {
        NodeContext context;
        context.isPrivate = true;

        InclusionPolicy policy;
        policy.includePrivate = false;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == false);
    }

    SECTION("Private node is included when policy allows")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;

        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.includePrivateFunction = true;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == true);
    }

    SECTION("Pure virtual functions are always included")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isPureVirtual = true;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = false;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == true);
    }
}

