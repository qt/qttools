// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_SORTING_H
#define QDOC_IR_SORTING_H

#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

namespace Sorting {

[[nodiscard]] Qt::SortOrder parseSortOrder(const QString &directive);

} // namespace Sorting

QT_END_NAMESPACE

#endif // QDOC_IR_SORTING_H
