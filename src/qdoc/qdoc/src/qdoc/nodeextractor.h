// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_NODEEXTRACTOR_H
#define QDOC_NODEEXTRACTOR_H

#include "ir/pagemetadata.h"

QT_BEGIN_NAMESPACE

class Aggregate;
class Node;
class PageNode;

namespace NodeExtractor {

IR::PageMetadata extractPageMetadata(const PageNode *pn);
QList<IR::SectionIR> extractSummarySections(const Aggregate *aggregate);
IR::MemberIR extractMemberIR(const Node *node);

} // namespace NodeExtractor

QT_END_NAMESPACE

#endif // QDOC_NODEEXTRACTOR_H
