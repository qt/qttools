// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module ComparisonTable
    \title Comparison Table
    \brief A module documenting comparison table cases.
*/

/*!
    \class SelfAndOthers
    \inmodule ComparisonTable
    \brief Compares with itself and with other types.

    \compares equality
    \compareswith equality SelfAndOthers* std::nullptr_t
    Compares against raw pointers and null.
    \endcompareswith
*/
class SelfAndOthers
{
public:
    SelfAndOthers();
};

/*!
    \class SelfOnly
    \inmodule ComparisonTable
    \brief Compares only with itself.

    \compares strong
*/
class SelfOnly
{
public:
    SelfOnly();
};

/*!
    \class OthersOnly
    \inmodule ComparisonTable
    \brief Compares only with other types.

    \compareswith partial int double
    \endcompareswith
*/
class OthersOnly
{
public:
    OthersOnly();
};
