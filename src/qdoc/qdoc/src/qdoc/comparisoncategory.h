// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef COMPARISONCATEGORY_H
#define COMPARISONCATEGORY_H

#include <string>

QT_BEGIN_NAMESPACE

enum struct ComparisonCategory : unsigned char {
    None,
    Strong,
    Weak,
    Partial,
    Equality
};

static inline std::string comparisonCategoryAsString(ComparisonCategory category)
{
    switch (category) {
    case ComparisonCategory::Strong:
        return "strong";
    case ComparisonCategory::Weak:
        return "weak";
    case ComparisonCategory::Partial:
        return "partial";
    case ComparisonCategory::Equality:
        return "equality";
    case ComparisonCategory::None:
        [[fallthrough]];
    default:
        break;
    }
    return {};
}

static inline ComparisonCategory comparisonCategoryFromString(const std::string &string)
{
    if (string == "strong")
        return ComparisonCategory::Strong;
    if (string == "weak")
        return ComparisonCategory::Weak;
    if (string == "partial")
        return ComparisonCategory::Partial;
    if (string == "equality")
        return ComparisonCategory::Equality;

    return ComparisonCategory::None;
}

QT_END_NAMESPACE

#endif // COMPARISONCATEGORY_H
