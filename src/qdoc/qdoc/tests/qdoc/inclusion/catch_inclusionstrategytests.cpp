// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>

#include <qdoc/inclusionflags.h>
#include <qdoc/inclusionpolicy.h>
#include <qdoc/nodecontext.h>
#include <qdoc/inclusionfilter.h>

TEST_CASE("NodeContext correctly captures internal status", "[NodeContext]")
{
    SECTION("Default NodeContext has isInternal false")
    {
        NodeContext context;
        REQUIRE(context.isInternal == false);
    }

    SECTION("NodeContext can be set to internal")
    {
        NodeContext context;
        context.isInternal = true;
        REQUIRE(context.isInternal == true);
    }

    SECTION("NodeContext preserves all fields when internal is set")
    {
        NodeContext context;
        context.type = NodeType::Class;
        context.isPrivate = true;
        context.isInternal = true;
        context.isPureVirtual = false;

        REQUIRE(context.type == NodeType::Class);
        REQUIRE(context.isPrivate == true);
        REQUIRE(context.isInternal == true);
        REQUIRE(context.isPureVirtual == false);
    }

    SECTION("NodeContext with private but not internal")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = false;

        REQUIRE(context.isPrivate == true);
        REQUIRE(context.isInternal == false);
    }

    SECTION("NodeContext with internal but not private")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false;
        context.isInternal = true;

        REQUIRE(context.isPrivate == false);
        REQUIRE(context.isInternal == true);
    }

    SECTION("NodeContext with both private and internal")
    {
        NodeContext context;
        context.type = NodeType::Class;
        context.isPrivate = true;
        context.isInternal = true;

        REQUIRE(context.isPrivate == true);
        REQUIRE(context.isInternal == true);
    }
}

TEST_CASE("NodeContext toFlags() behavior", "[NodeContext]")
{
    SECTION("Non-private node returns empty flags regardless of internal status")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false;
        context.isInternal = false;

        auto flags = context.toFlags();
        REQUIRE(flags == InclusionFlags{});

        context.isInternal = true;
        flags = context.toFlags();
        REQUIRE(flags == InclusionFlags{});
    }

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
        context.isInternal = false;

        InclusionPolicy policy;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == true);
    }

    SECTION("Private node is excluded by default")
    {
        NodeContext context;
        context.isPrivate = true;
        context.isInternal = false;

        InclusionPolicy policy;
        policy.includePrivate = false;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == false);
    }

    SECTION("Private node is included when policy allows")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = false;

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
        context.isInternal = false;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = false;

        REQUIRE(InclusionFilter::isIncluded(policy, context) == true);
    }
}

