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
    SECTION("Non-private, non-internal node returns empty flags")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false;
        context.isInternal = false;

        auto flags = context.toFlags();
        REQUIRE(flags == InclusionFlags{});
    }

    SECTION("Non-private but internal node returns Internal flag")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false;
        context.isInternal = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Internal) == InclusionFlag::Internal);
        REQUIRE((flags & InclusionFlag::Private) != InclusionFlag::Private);
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

    SECTION("Internal and private function node returns both flags")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateFunction) == InclusionFlag::PrivateFunction);
        REQUIRE((flags & InclusionFlag::Internal) == InclusionFlag::Internal);
    }

    SECTION("Internal and private class node returns both flags")
    {
        NodeContext context;
        context.type = NodeType::Class;
        context.isPrivate = true;
        context.isInternal = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateType) == InclusionFlag::PrivateType);
        REQUIRE((flags & InclusionFlag::Internal) == InclusionFlag::Internal);
    }

    SECTION("Internal-only node returns only Internal flag")
    {
        NodeContext context;
        context.type = NodeType::Class;
        context.isPrivate = false;
        context.isInternal = true;

        auto flags = context.toFlags();
        REQUIRE((flags & InclusionFlag::Internal) == InclusionFlag::Internal);
        REQUIRE((flags & InclusionFlag::Private) != InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateType) != InclusionFlag::PrivateType);
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
        REQUIRE(policy.showInternal == false);
    }

    SECTION("InclusionPolicy with custom values")
    {
        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.includePrivateFunction = true;
        policy.includePrivateType = true;
        policy.includePrivateVariable = true;
        policy.showInternal = true;

        REQUIRE(policy.includePrivate == true);
        REQUIRE(policy.includePrivateFunction == true);
        REQUIRE(policy.includePrivateType == true);
        REQUIRE(policy.includePrivateVariable == true);
        REQUIRE(policy.showInternal == true);
    }

    SECTION("InclusionPolicy toFlags() with showInternal")
    {
        InclusionPolicy policy;
        policy.showInternal = true;

        auto flags = policy.toFlags();
        REQUIRE((flags & InclusionFlag::Internal) == InclusionFlag::Internal);
        REQUIRE((flags & InclusionFlag::Private) != InclusionFlag::Private);
    }

    SECTION("InclusionPolicy toFlags() without showInternal")
    {
        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.showInternal = false;

        auto flags = policy.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::Internal) != InclusionFlag::Internal);
    }

    SECTION("InclusionPolicy toFlags() with both private and internal")
    {
        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.includePrivateFunction = true;
        policy.showInternal = true;

        auto flags = policy.toFlags();
        REQUIRE((flags & InclusionFlag::Private) == InclusionFlag::Private);
        REQUIRE((flags & InclusionFlag::PrivateFunction) == InclusionFlag::PrivateFunction);
        REQUIRE((flags & InclusionFlag::Internal) == InclusionFlag::Internal);
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

TEST_CASE("InclusionFilter::isReimplementedMemberVisible functionality", "[InclusionFilter]")
{
    SECTION("Public reimplemented member is always visible")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false;
        context.isInternal = false;

        InclusionPolicy policy;
        policy.showInternal = false;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Public internal reimplemented member is visible despite internal status")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false;
        context.isInternal = true;

        InclusionPolicy policy;
        policy.showInternal = false;

        // This is the key test - public reimplemented members should be visible
        // even when marked internal and showInternal is false
        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Protected reimplemented member is always visible")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false; // protected is considered non-private
        context.isInternal = false;

        InclusionPolicy policy;
        policy.showInternal = false;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Protected internal reimplemented member is visible despite internal status")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = false; // protected is considered non-private
        context.isInternal = true;

        InclusionPolicy policy;
        policy.showInternal = false;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Pure virtual private reimplemented member is always visible")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = false;
        context.isPureVirtual = true;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = false;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Pure virtual private internal reimplemented member is visible")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = true;
        context.isPureVirtual = true;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = false;
        policy.showInternal = false;

        // Pure virtual should override internal status for reimplemented members
        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Private reimplemented member follows private inclusion policy")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = false;
        context.isPureVirtual = false;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = false;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == false);
    }

    SECTION("Private reimplemented member included when private policy allows")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = false;
        context.isPureVirtual = false;

        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.includePrivateFunction = true;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Private internal reimplemented member ignores internal status")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = true;
        context.isPureVirtual = false;

        InclusionPolicy policy;
        policy.includePrivate = true;
        policy.includePrivateFunction = true;
        policy.showInternal = false; // This should be ignored

        // Should follow private policy, not internal policy
        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }

    SECTION("Private internal reimplemented member excluded when private policy disallows")
    {
        NodeContext context;
        context.type = NodeType::Function;
        context.isPrivate = true;
        context.isInternal = true;
        context.isPureVirtual = false;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = false;
        policy.showInternal = true; // This should be ignored for reimplemented members

        // Should follow private policy, not internal policy
        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == false);
    }

    SECTION("Private variable reimplemented member follows private variable policy")
    {
        NodeContext context;
        context.type = NodeType::Variable;
        context.isPrivate = true;
        context.isInternal = true;
        context.isPureVirtual = false;

        InclusionPolicy policy;
        policy.includePrivate = false;
        policy.includePrivateFunction = true; // This doesn't apply to variables
        policy.includePrivateVariable = true; // This does
        policy.showInternal = false;

        REQUIRE(InclusionFilter::isReimplementedMemberVisible(policy, context) == true);
    }
}

