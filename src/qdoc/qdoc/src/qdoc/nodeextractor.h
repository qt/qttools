// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_NODEEXTRACTOR_H
#define QDOC_NODEEXTRACTOR_H

#include "ir/pagemetadata.h"

QT_BEGIN_NAMESPACE

class PageNode;

namespace NodeExtractor {

IR::PageMetadata extractPageMetadata(const PageNode *pn, const QString &format);

} // namespace NodeExtractor

QT_END_NAMESPACE

#endif // QDOC_NODEEXTRACTOR_H
