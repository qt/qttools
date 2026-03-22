// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_NODEEXTRACTOR_H
#define QDOC_NODEEXTRACTOR_H

#include "ir/pagemetadata.h"

#include <optional>

QT_BEGIN_NAMESPACE

class Aggregate;
class CollectionNode;
class HrefResolver;
class Node;
class PageNode;
class QmlTypeNode;

namespace NodeExtractor {

IR::PageMetadata extractPageMetadata(const PageNode *pn, const HrefResolver *hrefResolver = nullptr);
IR::QmlTypeData extractQmlTypeData(const QmlTypeNode *qcn, const HrefResolver *hrefResolver = nullptr);
IR::CollectionData extractCollectionData(const CollectionNode *cn, const HrefResolver *hrefResolver = nullptr);
QList<IR::SectionIR> extractSummarySections(const Aggregate *aggregate, const HrefResolver *hrefResolver = nullptr);
IR::MemberIR extractMemberIR(const Node *node, const HrefResolver *hrefResolver = nullptr, const Node *relative = nullptr);
std::optional<IR::AllMembersIR> extractAllMembersIR(const PageNode *pn, const HrefResolver *hrefResolver = nullptr);

} // namespace NodeExtractor

QT_END_NAMESPACE

#endif // QDOC_NODEEXTRACTOR_H
