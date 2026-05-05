// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "listexpander.h"

#include "contentblock.h"
#include "listplaceholder.h"

#include <optional>
#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

/*!
    \class IR::ListExpander
    \internal
    \brief Post-IR pass that replaces ListPlaceholder blocks with
    populated Catalog subtrees.

    ListExpander walks a block tree in place and, for every
    ListPlaceholder encountered, reads its variant attribute and
    produces a Catalog wrapper whose children reuse the existing
    Table, SectionHeading, List, ListItem, and Link block types.
    Entry data comes through the ListExpanderCallbacks struct, which
    the driver populates with lambdas bound to a CatalogEntrySource.
    When a callback returns an empty set the placeholder is removed
    and \c{ListExpanderCallbacks::onEmpty} is invoked so the driver
    can warn the author with whatever attribution it has — the
    expander itself logs nothing, keeping QDocLib free of
    driver-layer logging categories.

    The expander is QDocLib-pure: it has no Node or QDocDatabase
    dependencies and receives the relative-node pointer purely as
    opaque state forwarded to the callbacks.

    This first commit lands the dispatch skeleton and the
    annotated-classes variant. The remaining variants
    (annotated-examples, compact-classes, annotated-group) arrive in
    the follow-up commit on the same series.

    \sa CatalogEntry, CatalogEntryGroup, ListExpanderCallbacks,
        ListPlaceholderVariant
*/

/*!
    \struct IR::ListExpanderCallbacks
    \internal
    \brief Callback bundle that injects entry data and warning
    handling into ListExpander.

    Each data callback is the source for one ListPlaceholder variant;
    \c{onEmpty} is the warning hook invoked when an enumeration
    yields no entries. The driver-wiring commit binds these to
    CatalogEntrySource methods and a \c{qCWarning(lcQdoc)} call; the
    QDocLib expander itself has no knowledge of the bound types
    beyond the value-semantics CatalogEntry it consumes.
*/

namespace IR {

namespace {

InlineContent makeLink(const QString &name, const QString &href)
{
    InlineContent link;
    link.type = InlineType::Link;
    link.href = href;
    InlineContent nameInline;
    nameInline.type = InlineType::Text;
    nameInline.text = name;
    link.children.append(nameInline);
    return link;
}

InlineContent makeText(const QString &text)
{
    InlineContent t;
    t.type = InlineType::Text;
    t.text = text;
    return t;
}

ContentBlock makeCell(QList<InlineContent> inlines)
{
    ContentBlock cell;
    cell.type = BlockType::TableCell;
    cell.inlineContent = std::move(inlines);
    return cell;
}

ContentBlock makeRow(QList<ContentBlock> cells)
{
    ContentBlock row;
    row.type = BlockType::TableRow;
    row.children = std::move(cells);
    return row;
}

ContentBlock makeAnnotatedTable(const QList<CatalogEntry> &entries)
{
    ContentBlock table;
    table.type = BlockType::Table;
    table.attributes["style"_L1] = u"annotated"_s;

    for (const CatalogEntry &e : entries) {
        QList<InlineContent> nameInlines;
        if (!e.href.isEmpty())
            nameInlines.append(makeLink(e.name, e.href));
        else
            nameInlines.append(makeText(e.name));

        QList<InlineContent> briefInlines;
        briefInlines.append(makeText(e.brief));

        QList<ContentBlock> cells;
        cells.append(makeCell(std::move(nameInlines)));
        cells.append(makeCell(std::move(briefInlines)));

        ContentBlock row = makeRow(std::move(cells));
        // Surface the per-entry metadata CatalogEntrySource extracts.
        // Templates can render \since stamps next to the name and
        // strike-through or annotate deprecated rows. Empty since and
        // non-deprecated rows omit the keys to keep JSON output lean.
        if (!e.since.isEmpty())
            row.attributes["since"_L1] = e.since;
        if (e.isDeprecated)
            row.attributes["deprecated"_L1] = true;
        table.children.append(std::move(row));
    }
    return table;
}

ContentBlock makeCatalog(ListPlaceholderVariant variant)
{
    ContentBlock catalog;
    catalog.type = BlockType::Catalog;
    catalog.attributes["variant"_L1] = toString(variant);
    return catalog;
}

Qt::SortOrder readSortOrder(const ContentBlock &placeholder)
{
    return placeholder.attributes.value("sort"_L1).toString() == u"descending"_s
            ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void notifyEmpty(const ListExpanderCallbacks &cb, const QString &argument,
                 ListPlaceholderVariant variant)
{
    if (cb.onEmpty)
        cb.onEmpty(argument, variant);
}

std::optional<ContentBlock> expandAnnotatedClasses(
        const ListExpanderCallbacks &cb, const ContentBlock &placeholder,
        const Node *relative)
{
    const QString argument = placeholder.attributes.value("argument"_L1).toString();
    QList<CatalogEntry> entries = cb.collectCppClasses
            ? cb.collectCppClasses(relative, readSortOrder(placeholder))
            : QList<CatalogEntry>{};

    if (entries.isEmpty()) {
        notifyEmpty(cb, argument, ListPlaceholderVariant::AnnotatedClasses);
        return std::nullopt;
    }

    ContentBlock catalog = makeCatalog(ListPlaceholderVariant::AnnotatedClasses);
    catalog.children.append(makeAnnotatedTable(entries));
    return catalog;
}

std::optional<ContentBlock> expandPlaceholder(
        const ListExpanderCallbacks &cb, const ContentBlock &placeholder,
        const Node *relative)
{
    const auto variant = parseListPlaceholderVariant(
            placeholder.attributes.value("variant"_L1).toString());
    if (!variant)
        return std::nullopt;

    switch (*variant) {
    case ListPlaceholderVariant::AnnotatedClasses:
        return expandAnnotatedClasses(cb, placeholder, relative);
    case ListPlaceholderVariant::AnnotatedExamples:
    case ListPlaceholderVariant::CompactClasses:
    case ListPlaceholderVariant::AnnotatedGroup:
        // Implemented in the follow-up commit. Until then the
        // dispatch drops the placeholder; existing legacy generators
        // continue rendering these atoms via their own pipelines.
        return std::nullopt;
    }
    Q_UNREACHABLE_RETURN(std::nullopt);
}

} // anonymous namespace

ListExpander::ListExpander(ListExpanderCallbacks callbacks)
    : m_callbacks(std::move(callbacks))
{
}

/*!
    Walks \a blocks in place, replacing every BlockType::ListPlaceholder
    with a populated BlockType::Catalog subtree, or removing the
    placeholder when the enumeration is empty. Empty enumerations
    invoke \c{ListExpanderCallbacks::onEmpty} (when set) so the driver
    can warn the author. Recurses into \c{block.children} so placeholders
    nested inside sections or other container blocks are expanded too.

    The \a relative pointer is forwarded as opaque state to the
    callbacks, which use it to resolve relative-aware display names
    and hrefs.
*/
void ListExpander::expand(QList<ContentBlock> &blocks, const Node *relative)
{
    for (qsizetype i = 0; i < blocks.size(); ) {
        if (blocks[i].type == BlockType::ListPlaceholder) {
            auto expanded = expandPlaceholder(m_callbacks, blocks[i], relative);
            if (expanded) {
                blocks.replace(i, std::move(*expanded));
                ++i;
            } else {
                blocks.removeAt(i);
            }
        } else {
            expand(blocks[i].children, relative);
            ++i;
        }
    }
}

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
