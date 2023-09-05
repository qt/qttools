// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \enum ComparisonCategory
    \internal

    \value None No comparison is defined.
    \value Strong Strong comparison is defined, see std::strong_ordering.
    \value Weak Weak comparison is defined, see std::weak_ordering.
    \value Partial A partial ordering is defined, see std::partial_ordering.
    \value Equality Only (in)equality comparison is defined.
*/

/*!
    \fn static inline std::string comparisonCategoryAsString(const ComparisonCategory &category)
    \internal

    Returns a string representation of the comparison category \a category.
*/

/*!
    \fn static ComparisonCategory comparisonCategoryFromString(const std::string &string)
    \internal

    Returns a matching comparison category for a \a string representation, or
    ComparisonCategory::None for an unknown category string.
*/
