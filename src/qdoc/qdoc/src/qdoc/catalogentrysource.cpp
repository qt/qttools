// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "catalogentrysource.h"

#include "collectionnode.h"
#include "doc.h"
#include "hrefresolver.h"
#include "inclusionfilter.h"
#include "node.h"
#include "qdocdatabase.h"
#include "text.h"
#include "textutils.h"

#include <algorithm>
#include <optional>
#include <variant>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

/*!
    \class CatalogEntrySource
    \internal
    \brief Driver-side extraction surface that reads \\generatelist
    and \\annotatedlist data from QDocDatabase and returns the results
    as QDocLib value types.

    CatalogEntrySource holds the driver-layer collaborators: the QDoc
    database, the href resolver, and the current inclusion policy.
    Each public method corresponds to one catalog variant — the flat
    annotated classes list, the per-module grouped examples list, the
    compact classes list with an optional root-name filter, and the
    group-members list. All four return CatalogEntry or
    CatalogEntryGroup values with hrefs pre-resolved and inclusion
    filtering applied at the boundary, ready for the QDocLib
    list-expander pass to splice into the block tree.
*/

namespace {

QString extractBrief(const Node *node)
{
    // Mirrors HtmlGenerator::generateAnnotatedList. Text pages prefer
    // reconstitutedBrief() because their brief usually arrives that
    // way from index files and there's no entity-name prefix worth
    // stripping. Non-text pages prefer trimmedBriefText(node->name())
    // because their authored brief typically opens with the entity's
    // own name (e.g. "QFoo is a class that...") which the trimmer
    // removes for cleaner display.
    if (node->isTextPageNode()) {
        const QString &reconstituted = node->reconstitutedBrief();
        if (!reconstituted.isEmpty())
            return reconstituted;
        return node->doc().briefText().toString();
    }
    const Text brief = node->doc().trimmedBriefText(node->name());
    if (!brief.isEmpty())
        return brief.toString();
    return node->reconstitutedBrief();
}

QString resolveEntryHref(const HrefResolver &resolver, const Node *target,
                         const Node *relative)
{
    // Always go through HrefResolver. It already handles Node::url()
    // correctly: external URLs (those containing "://") pass through
    // unchanged, while non-external urls have their baked-in
    // ../<module>/ prefix stripped and the path recomputed from the
    // current output layout. Short-circuiting on a non-empty url()
    // here would re-introduce the cross-module href bug that the
    // path-based resolver was added to fix.
    auto result = resolver.hrefForNode(target, relative);
    if (std::get_if<HrefSuppressReason>(&result))
        return {};
    return std::get<QString>(std::move(result));
}

std::optional<IR::CatalogEntry> buildEntry(const Node *node,
                                           const Node *relative,
                                           const HrefResolver &resolver,
                                           const InclusionPolicy &policy)
{
    const NodeContext context = node->createContext();
    if (!InclusionFilter::isIncluded(policy, context))
        return std::nullopt;

    const QString href = resolveEntryHref(resolver, node, relative);
    if (href.isEmpty())
        return std::nullopt;

    IR::CatalogEntry entry;
    entry.name = node->fullName(relative);
    entry.href = href;
    entry.brief = extractBrief(node);
    entry.since = node->since();
    entry.isDeprecated = node->isDeprecated();
    return entry;
}

void sortEntries(QList<IR::CatalogEntry> &entries, Qt::SortOrder order)
{
    auto ascending = [](const IR::CatalogEntry &a, const IR::CatalogEntry &b) {
        return QString::localeAwareCompare(a.name, b.name) < 0;
    };
    if (order == Qt::DescendingOrder) {
        std::sort(entries.begin(), entries.end(),
                  [&](const IR::CatalogEntry &a, const IR::CatalogEntry &b) {
                      return ascending(b, a);
                  });
    } else {
        std::sort(entries.begin(), entries.end(), ascending);
    }
}

} // namespace

CatalogEntrySource::CatalogEntrySource(QDocDatabase &qdb,
                                       const HrefResolver &hrefResolver,
                                       InclusionPolicy policy)
    : m_qdb(qdb),
      m_hrefResolver(hrefResolver),
      m_inclusionPolicy(std::move(policy))
{
}

/*!
    Returns the catalog entries for the C++ classes index, sorted
    according to \a sortOrder. The \a relative node is forwarded to
    each entry so display names can strip common prefixes.

    Suppressed nodes — those denied by the inclusion policy or whose
    href cannot be resolved — are dropped before sorting so excluded
    entries never enter the ordered output.
*/
QList<IR::CatalogEntry> CatalogEntrySource::collectCppClasses(
        const Node *relative, Qt::SortOrder sortOrder) const
{
    QList<IR::CatalogEntry> entries;
    const NodeMultiMap &nmm = m_qdb.getCppClasses();
    entries.reserve(nmm.size());

    for (const Node *node : nmm.values()) {
        auto entry = buildEntry(node, relative, m_hrefResolver,
                                m_inclusionPolicy);
        if (entry)
            entries.append(*entry);
    }

    sortEntries(entries, sortOrder);
    return entries;
}

/*!
    Returns the example pages grouped by their owning module. Groups
    appear in alphabetical key order — \c{NodeMultiMap} is a
    \c{QMultiMap}, so \c{uniqueKeys()} is sorted by definition — and
    this matches \c{HtmlGenerator::generateAnnotatedLists} which
    iterates the same multimap's unique keys for the same data.
    Entries within each group are sorted ascending. Groups that end
    up empty after inclusion filtering are dropped.
*/
QList<IR::CatalogEntryGroup> CatalogEntrySource::collectExamplesGrouped(
        const Node *relative) const
{
    QList<IR::CatalogEntryGroup> groups;
    const NodeMultiMap &nmm = m_qdb.getExamples();
    const QList<QString> keys = nmm.uniqueKeys();

    for (const QString &key : keys) {
        IR::CatalogEntryGroup group;
        group.label = key;
        group.anchorId = TextUtils::asAsciiPrintable(key);

        for (const Node *node : nmm.values(key)) {
            auto entry = buildEntry(node, relative, m_hrefResolver,
                                    m_inclusionPolicy);
            if (entry)
                group.entries.append(*entry);
        }
        sortEntries(group.entries, Qt::AscendingOrder);

        if (!group.entries.isEmpty())
            groups.append(std::move(group));
    }

    return groups;
}

/*!
    Returns the entries for the compact classes list, optionally
    filtered by \a rootName. An empty rootName returns every class;
    a non-empty rootName keeps only classes whose unqualified name
    begins with the filter, matched case-insensitively to follow the
    legacy \c{generateCompactList} behavior of bucketing on the last
    \c{::} segment of each class name.

    Entries are always sorted ascending. The placeholder's sort
    directive is intentionally ignored here: the legacy compact-list
    pipeline alphabetizes into 37 paragraph buckets, so descending
    order has never been a meaningful option for this variant.
*/
QList<IR::CatalogEntry> CatalogEntrySource::collectCompactClasses(
        const Node *relative, const QString &rootName) const
{
    QList<IR::CatalogEntry> entries;
    const NodeMultiMap &nmm = m_qdb.getCppClasses();
    entries.reserve(nmm.size());

    for (const Node *node : nmm.values()) {
        if (!rootName.isEmpty()
            && !node->name().startsWith(rootName, Qt::CaseInsensitive)) {
            continue;
        }
        auto entry = buildEntry(node, relative, m_hrefResolver,
                                m_inclusionPolicy);
        if (entry)
            entries.append(*entry);
    }

    sortEntries(entries, Qt::AscendingOrder);
    return entries;
}

/*!
    Returns the members of the named group, resolved through
    QDocDatabase::getCollectionNode with NodeType::Group. An unknown
    or empty group yields an empty list, leaving the caller to warn
    once rather than raising mid-enumeration. Entries are sorted
    according to \a sortOrder.
*/
QList<IR::CatalogEntry> CatalogEntrySource::collectGroupMembers(
        const Node *relative, const QString &groupName,
        Qt::SortOrder sortOrder) const
{
    QList<IR::CatalogEntry> entries;
    if (groupName.isEmpty())
        return entries;

    const CollectionNode *cn = m_qdb.getCollectionNode(groupName, NodeType::Group);
    if (!cn)
        return entries;

    for (const Node *node : cn->members()) {
        auto entry = buildEntry(node, relative, m_hrefResolver,
                                m_inclusionPolicy);
        if (entry)
            entries.append(*entry);
    }

    sortEntries(entries, sortOrder);
    return entries;
}

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
