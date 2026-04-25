// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "sorting.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

/*!
    \namespace Sorting
    \internal
    \brief Pure text-to-enum helpers for sort directives used by
    \\generatelist and \\annotatedlist atoms.

    These helpers live in QDocLib so that the QDocLib-pure content
    builder can parse sort directives without pulling in the driver-layer
    Generator header. The legacy Generator::sortOrder member delegates
    here, preserving legacy behavior at all call sites.
*/

/*!
    \internal
    Parses a QDoc sort directive into a Qt::SortOrder.

    Returns \c Qt::DescendingOrder when \a directive is \c "descending"
    (exact match, case-sensitive). Any other value, including the empty
    string, returns \c Qt::AscendingOrder. This matches the legacy
    behavior of Generator::sortOrder, which accepts the atom's raw
    second-string argument even when it is the command's primary
    argument rather than a bracketed sort directive.
*/
Qt::SortOrder Sorting::parseSortOrder(const QString &directive)
{
    return (directive == "descending"_L1) ? Qt::DescendingOrder : Qt::AscendingOrder;
}

QT_END_NAMESPACE
