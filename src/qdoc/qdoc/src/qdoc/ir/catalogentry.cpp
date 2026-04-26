// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "catalogentry.h"

QT_BEGIN_NAMESPACE

/*!
    \struct IR::CatalogEntry
    \internal
    \brief A single entry in a catalog page such as the classes index,
    the examples index, or a group's members table.

    CatalogEntry carries the fields that legacy HtmlGenerator reads
    when rendering an annotated-list row: the display name, the
    pre-resolved link target, and a brief description. A since-version
    stamp and a deprecation flag round out the shape so catalog
    renderers can mark versioned or deprecated entries without
    reaching back into Node state.

    CatalogEntry is a QDocLib value type with no Node, Generator,
    Config, or Tree dependencies. Driver-side extraction populates
    it; the list-expander pass consumes it.

    \sa CatalogEntryGroup
*/

/*!
    \variable IR::CatalogEntry::name
    Display name of the entry, as returned by node->fullName(relative)
    at extraction time. The name is relative-aware: common prefixes
    with the current page are stripped when the page context allows.
*/

/*!
    \variable IR::CatalogEntry::href
    Pre-resolved link target. Resolved by HrefResolver at extraction
    time so catalog expansion avoids a second pass through the link
    resolver. Empty when the entry is suppressed by the inclusion
    policy.
*/

/*!
    \variable IR::CatalogEntry::brief
    Plain-text brief description. For code-backed nodes such as
    classes, this comes from Doc::trimmedBriefText; for text pages
    such as example index pages, it comes from
    Node::reconstitutedBrief with a fallback to Doc::briefText. The
    brief is flattened to text at extraction time because legacy
    catalog rendering doesn't apply inline formatting within catalog
    tables.
*/

/*!
    \variable IR::CatalogEntry::since
    The version string declared via \\since on the documented entity,
    or empty when no since stamp is present.
*/

/*!
    \variable IR::CatalogEntry::isDeprecated
    True when the documented entity is marked deprecated. Catalog
    renderers can use this to apply a strike-through style or to
    filter the entry, matching whichever behavior the consuming
    template prescribes.
*/

/*!
    \struct IR::CatalogEntryGroup
    \internal
    \brief A labeled group of catalog entries, used by variants such
    as \\generatelist annotatedexamples that partition their entries
    by module or by category.

    CatalogEntryGroup pairs a display label and an anchor identifier
    with a list of entries. The list-expander pass emits one
    SectionHeading plus one Table per group when rendering a grouped
    catalog variant.

    \sa CatalogEntry
*/

/*!
    \variable IR::CatalogEntryGroup::label
    Display label for the group. For the annotated-examples variant,
    this is the owning module's index title; future grouped variants
    may use a different grouping key.
*/

/*!
    \variable IR::CatalogEntryGroup::anchorId
    A slug derived from the label, suitable for use as an HTML anchor
    and for cross-page navigation. Pre-computed at extraction time so
    rendering stays free of text munging.
*/

/*!
    \variable IR::CatalogEntryGroup::entries
    The list of CatalogEntry values belonging to this group, already
    sorted per the originating \\generatelist directive.
*/

QT_END_NAMESPACE
