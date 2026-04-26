// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_LISTPLACEHOLDER_H
#define QDOC_IR_LISTPLACEHOLDER_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

namespace IR {

enum class ListPlaceholderVariant : unsigned char {
    AnnotatedGroup,
    AnnotatedExamples,
    AnnotatedClasses,
    CompactClasses
};

[[nodiscard]] QString toString(ListPlaceholderVariant variant);

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_LISTPLACEHOLDER_H
