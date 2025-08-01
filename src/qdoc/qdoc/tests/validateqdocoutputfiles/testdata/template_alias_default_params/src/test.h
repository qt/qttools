// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

#pragma once

/*!
    \namespace TestAliases
    \inmodule TemplateAliasModule
    \brief Namespace containing template alias tests.
*/
namespace TestAliases {

/*!
    \typealias TestAliases::Foo

    Template alias with default parameter. This should match correctly.

    This type alias demonstrates QDoc's ability to handle template aliases
    with default parameter values.
*/
template <int N = 1>
using Foo = int[N];

/*!
    \typealias TestAliases::Bar

    Template alias with multiple parameters and defaults.

    This type alias has both a type parameter and a non-type parameter
    with a default value.
*/
template <typename T, int N = 5>
using Bar = T[N];

/*!
    \typealias TestAliases::SimpleAlias

    Simple alias without template parameters.

    This demonstrates that the fix doesn't break non-template aliases.
*/
using SimpleAlias = int;

} // namespace TestAliases

