// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <catch/catch.hpp>
#include <qdoc/qdocdatabase.h>

#include <type_traits>

QT_USE_NAMESPACE

static_assert(std::is_convertible_v<FindFlag, FindFlags>);
static_assert(!std::is_convertible_v<int, FindFlags>);

TEST_CASE("FindFlags preserve combined search criteria", "[FindFlags]")
{
    const FindFlags flags = SearchBaseClasses | QmlAttachedProperties;
    REQUIRE(flags.testFlag(SearchBaseClasses));
    REQUIRE(flags.testFlag(QmlAttachedProperties));
    REQUIRE(!flags.testFlag(SearchEnumValues));
    REQUIRE(FindFlags::fromInt(flags.toInt()) == flags);
}
