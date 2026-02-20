// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "contentbuilder.h"

#include "../atom.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \class IR::ContentBuilder
    \internal
    \brief Converts Atom chains to QList<IR::ContentBlock> trees.

    ContentBuilder walks a linked list of Atom nodes (QDoc's internal
    documentation representation) and produces a structured tree of
    ContentBlock and InlineContent values suitable for template rendering.

    Handled atom types:
    \list
        \li ParaLeft, ParaRight -- Paragraph blocks.
        \li String -- Text inline content.
        \li C -- Inline code spans.
        \li SectionLeft, SectionRight -- Section containers.
        \li SectionHeadingLeft, SectionHeadingRight -- Section headings with
            level.
        \li FormattingLeft, FormattingRight -- Bold, italic, teletype,
            underline, subscript, superscript, parameter, uicontrol, trademark,
            link, index, notranslate, span.
        \li BriefLeft, BriefRight -- Brief exclusion (skipped in body).
        \li Link, NavLink -- Explicit links with unresolved target.
        \li AutoLink, NavAutoLink -- Auto-linked type names with unresolved
            target.
        \li BR -- Line break inline.
        \li FormatIf, FormatElse, FormatEndif -- Format-conditional content.
        \li Nop -- No-operation (skipped).
        \li BaseName -- No-operation (skipped).
    \endlist

    ContentBuilder depends only on Atom (for reading the chain) and IR types
    (for producing output).

    \sa ContentBlock, InlineContent
*/

static InlineType formattingToInlineType(const QString &formatting)
{
    if (formatting == ATOM_FORMATTING_BOLD)
        return InlineType::Bold;
    if (formatting == ATOM_FORMATTING_ITALIC)
        return InlineType::Italic;
    if (formatting == ATOM_FORMATTING_TELETYPE)
        return InlineType::Teletype;
    if (formatting == ATOM_FORMATTING_UNDERLINE)
        return InlineType::Underline;
    if (formatting == ATOM_FORMATTING_SUBSCRIPT)
        return InlineType::Subscript;
    if (formatting == ATOM_FORMATTING_SUPERSCRIPT)
        return InlineType::Superscript;
    if (formatting == ATOM_FORMATTING_PARAMETER)
        return InlineType::Parameter;
    return InlineType::Text;
}

/*!
    Constructs a ContentBuilder that evaluates FormatIf atoms against
    \a format. If \a format is empty, all FormatIf branches are skipped.
*/
ContentBuilder::ContentBuilder(const QString &format)
    : m_format(format)
{
}

/*!
    Walks the atom chain starting at \a firstAtom and returns a list
    of ContentBlock trees representing the structured documentation body.

    Returns an empty list if \a firstAtom is \nullptr.

    The builder is reset before processing, so a single ContentBuilder
    instance can be reused for multiple build() calls.
*/
QList<ContentBlock> ContentBuilder::build(const Atom *firstAtom)
{
    m_result.clear();
    m_blockPath.clear();
    m_inlinePath.clear();
    m_inlineBaseDepths.clear();
    m_inBrief = false;
    m_inLink = false;

    if (!firstAtom)
        return {};

    processAtoms(firstAtom);

    // Malformed atom chain recovery: auto-close remaining blocks in
    // release builds, assert in debug to surface the source error.
    if (Q_UNLIKELY(!m_blockPath.isEmpty())) {
        Q_ASSERT_X(false, "ContentBuilder::build",
                    "Unclosed blocks at end of atom chain");
        while (!m_blockPath.isEmpty())
            closeBlock();
    }

    Q_ASSERT(m_inlinePath.isEmpty());
    Q_ASSERT(m_inlineBaseDepths.isEmpty());
    Q_ASSERT(!m_inLink);
    Q_ASSERT(!m_inBrief);

    return m_result;
}

/*!
    Walks the full atom chain starting at \a atom, building the
    content tree. Stray FormatElse and FormatEndif atoms outside
    any FormatIf context are ignored.
*/
void ContentBuilder::processAtoms(const Atom *atom)
{
    while (atom) {
        atom = processUntilBoundary(atom);
        if (!atom)
            return;
        // Stray FormatElse/FormatEndif at top level — skip.
        // Malformed chains (e.g. extra FormatElse) are treated as stray.
        atom = atom->next();
    }
}

/*!
    Builds content from atoms until a FormatElse, FormatEndif, or
    end of chain is reached. Returns the boundary atom, or \nullptr
    if the chain ends without one.
*/
const Atom *ContentBuilder::processUntilBoundary(const Atom *atom)
{
    while (atom) {
        if (atom->type() == Atom::FormatIf) {
            atom = processFormatIf(atom);
            continue;
        }
        if (atom->type() == Atom::FormatElse || atom->type() == Atom::FormatEndif)
            return atom;
        atom = dispatchAtom(atom);
        if (!atom)
            return nullptr;
        atom = atom->next();
    }
    return nullptr;
}

/*!
    Evaluates a FormatIf condition against the configured format.
    Builds content from the matching branch and skips the other.
    When no format is configured, both branches are skipped.

    Returns a pointer to the atom after FormatEndif.
*/
const Atom *ContentBuilder::processFormatIf(const Atom *atom)
{
    Q_ASSERT(atom->type() == Atom::FormatIf);

    if (m_format.isEmpty()) {
        atom = skipUntilBoundary(atom->next());
        if (!atom)
            return nullptr;
        if (atom->type() == Atom::FormatElse) {
            atom = skipUntilBoundary(atom->next());
            if (!atom)
                return nullptr;
        }
        if (atom->type() == Atom::FormatEndif)
            atom = atom->next();
        return atom;
    }

    const bool formatMatches = (atom->string().compare(m_format, Qt::CaseInsensitive) == 0);

    if (formatMatches)
        atom = processUntilBoundary(atom->next());
    else
        atom = skipUntilBoundary(atom->next());
    if (!atom)
        return nullptr;

    if (atom->type() == Atom::FormatElse) {
        if (formatMatches)
            atom = skipUntilBoundary(atom->next());
        else
            atom = processUntilBoundary(atom->next());
        if (!atom)
            return nullptr;
    }

    if (atom->type() == Atom::FormatEndif)
        atom = atom->next();

    return atom;
}

/*!
    Advances past atoms until a FormatElse, FormatEndif, or end of
    chain is reached, without building any content. Handles nested
    FormatIf blocks by skipping both branches.

    Returns the boundary atom, or \nullptr if the chain ends.
*/
const Atom *ContentBuilder::skipUntilBoundary(const Atom *atom)
{
    while (atom) {
        if (atom->type() == Atom::FormatIf) {
            atom = skipFormatIf(atom);
            continue;
        }
        if (atom->type() == Atom::FormatElse || atom->type() == Atom::FormatEndif)
            return atom;
        atom = atom->next();
    }
    return nullptr;
}

/*!
    Skips a complete FormatIf block (both branches) without building
    content.

    Returns a pointer to the atom after FormatEndif.
*/
const Atom *ContentBuilder::skipFormatIf(const Atom *atom)
{
    Q_ASSERT(atom->type() == Atom::FormatIf);
    atom = skipUntilBoundary(atom->next());
    if (!atom)
        return nullptr;
    if (atom->type() == Atom::FormatElse) {
        atom = skipUntilBoundary(atom->next());
        if (!atom)
            return nullptr;
    }
    if (atom->type() == Atom::FormatEndif)
        atom = atom->next();
    return atom;
}

/*!
    Dispatches a single atom to the content model. Returns the last
    atom consumed — usually \a atom itself, but some atom types may
    consume subsequent atoms.
*/
const Atom *ContentBuilder::dispatchAtom(const Atom *atom)
{
    if (atom->type() == Atom::BriefLeft) {
        m_inBrief = true;
        return atom;
    }
    if (atom->type() == Atom::BriefRight) {
        m_inBrief = false;
        return atom;
    }
    if (m_inBrief)
        return atom;

    switch (atom->type()) {

    case Atom::ParaLeft:
        openBlock(BlockType::Paragraph);
        break;

    case Atom::ParaRight:
        closeBlock();
        break;

    case Atom::String:
        addLeafInline(InlineType::Text, atom->string());
        break;

    case Atom::C:
        addLeafInline(InlineType::Code, atom->string());
        break;

    case Atom::AutoLink:
    case Atom::NavAutoLink: {
        InlineContent link;
        link.type = InlineType::Link;
        link.href = atom->string();
        link.children.append({ InlineType::Text, atom->string(), {}, {}, {} });
        addInline(std::move(link));
        break;
    }

    case Atom::Link:
    case Atom::NavLink: {
        if (Q_UNLIKELY(m_blockPath.isEmpty()))
            break;

        m_inLink = true;

        InlineContent link;
        link.type = InlineType::Link;
        link.href = atom->string();

        pushInlineContainer(std::move(link));

        // Link atoms are always followed by FormattingLeft("link");
        // skip it to avoid double-processing.
        if (atom->next() && atom->next()->type() == Atom::FormattingLeft
            && atom->next()->string() == ATOM_FORMATTING_LINK) {
            return atom->next();
        }
        break;
    }

    case Atom::FormattingLeft: {
        const QString &fmt = atom->string();

        if (fmt == ATOM_FORMATTING_INDEX || fmt.startsWith(u"span "_s))
            break;

        if (fmt == ATOM_FORMATTING_LINK)
            break;

        if (fmt == ATOM_FORMATTING_TRADEMARK || fmt == ATOM_FORMATTING_NOTRANSLATE)
            break;

        if (fmt == ATOM_FORMATTING_UICONTROL) {
            InlineContent bold;
            bold.type = InlineType::Bold;
            pushInlineContainer(std::move(bold));
            break;
        }

        InlineType type = formattingToInlineType(fmt);
        if (type == InlineType::Text)
            break;

        InlineContent container;
        container.type = type;
        pushInlineContainer(std::move(container));
        break;
    }

    case Atom::FormattingRight: {
        const QString &fmt = atom->string();

        if (fmt == ATOM_FORMATTING_LINK) {
            if (m_inLink) {
                const qsizetype base = m_inlineBaseDepths.isEmpty() ? 0 : m_inlineBaseDepths.last();
                if (m_inlinePath.size() > base) {
                    Q_ASSERT(resolveInline()->type == InlineType::Link);
                    m_inlinePath.removeLast();
                }
                m_inLink = false;
            }
            break;
        }

        if (fmt == ATOM_FORMATTING_INDEX || fmt == ATOM_FORMATTING_NOTRANSLATE
            || fmt.startsWith(u"span "_s) || fmt == ATOM_FORMATTING_TRADEMARK) {
            break;
        }

        const qsizetype base = m_inlineBaseDepths.isEmpty() ? 0 : m_inlineBaseDepths.last();
        if (m_inlinePath.size() > base)
            m_inlinePath.removeLast();
        break;
    }

    case Atom::SectionLeft:
        openBlock(BlockType::Section);
        break;

    case Atom::SectionRight:
        closeBlock();
        break;

    case Atom::SectionHeadingLeft: {
        QJsonObject attrs;
        attrs["level"_L1] = atom->string().toInt();
        openBlock(BlockType::SectionHeading, attrs);
        break;
    }

    case Atom::SectionHeadingRight:
        closeBlock();
        break;

    case Atom::BR:
        addLeafInline(InlineType::LineBreak, {});
        break;

    case Atom::Nop:
    case Atom::BaseName:
        break;

    default:
        break;
    }
    return atom;
}

/*!
    Opens a new block of type \a type with optional \a attrs.

    If the block path is empty, the block is added to the top-level
    result list. Otherwise it is added as a child of the current
    container block.
*/
void ContentBuilder::openBlock(BlockType type, QJsonObject attrs)
{
    ContentBlock block;
    block.type = type;
    block.attributes = std::move(attrs);

    m_inlineBaseDepths.append(m_inlinePath.size());

    if (m_blockPath.isEmpty()) {
        m_result.append(std::move(block));
        m_blockPath.append(m_result.size() - 1);
    } else {
        auto *parent = resolveBlock();
        parent->children.append(std::move(block));
        m_blockPath.append(parent->children.size() - 1);
    }
}

/*!
    Closes the current block by popping it from the block path.

    Verifies that the inline path depth matches the depth recorded
    when this block was opened (all formatting pairs balanced).
    In release builds, the inline path is restored to the expected
    depth as a safety measure against malformed atom chains.
*/
void ContentBuilder::closeBlock()
{
    if (!m_blockPath.isEmpty()) {
        if (Q_UNLIKELY(m_inlineBaseDepths.isEmpty())) {
            // openBlock() always pushes a base depth, so this indicates
            // a logic error in the builder. Assert in debug; recover in
            // release by clearing all state.
            Q_ASSERT_X(false, "ContentBuilder::closeBlock",
                        "m_inlineBaseDepths empty with non-empty m_blockPath");
            m_inlinePath.clear();
            m_blockPath.clear();
            m_inlineBaseDepths.clear();
            m_inLink = false;
            return;
        }
        const qsizetype expectedDepth = m_inlineBaseDepths.last();
        if (m_inLink && m_inlinePath.size() > expectedDepth)
            m_inLink = false;
        Q_ASSERT(m_inlinePath.size() == expectedDepth);
        m_inlinePath.resize(expectedDepth);
        m_inlineBaseDepths.removeLast();
        m_blockPath.removeLast();
    }
}

/*!
    Adds \a inline_ to the current block's inline content.

    If there is an active inline container (from FormattingLeft or
    Link atom), the inline is added to that container's children
    instead.

    If no block is open, the inline is dropped. This shouldn't happen
    with well-formed atom chains (text is always wrapped in
    ParaLeft/ParaRight), and is asserted in debug builds.
*/
void ContentBuilder::addInline(InlineContent inline_)
{
    if (!m_inlinePath.isEmpty()) {
        resolveInline()->children.append(std::move(inline_));
    } else if (!m_blockPath.isEmpty()) {
        resolveBlock()->inlineContent.append(std::move(inline_));
    } else {
        // Inline content without an enclosing block is dropped.
        // QDoc's atom chains always wrap text in ParaLeft/ParaRight,
        // so this path indicates a malformed chain.
        Q_ASSERT_X(false, "ContentBuilder::addInline",
                    "Inline content without an enclosing block");
    }
}

/*!
    Convenience method: creates a leaf InlineContent of the given
    \a type with \a text and appends it via addInline().
*/
void ContentBuilder::addLeafInline(InlineType type, const QString &text)
{
    InlineContent ic;
    ic.type = type;
    ic.text = text;
    addInline(std::move(ic));
}

/*!
    Pushes \a container as a new inline container. Subsequent
    addInline() and pushInlineContainer() calls will nest
    their content inside this container.

    Unlike addInline(), which only appends leaf inlines, this method
    also updates m_inlinePath to enable nesting. It respects the
    existing inline path: if we are already inside a Link or formatting
    container, the new container is added as a child of that container.
*/
void ContentBuilder::pushInlineContainer(InlineContent container)
{
    if (!m_inlinePath.isEmpty()) {
        auto *parent = resolveInline();
        parent->children.append(std::move(container));
        m_inlinePath.append(parent->children.size() - 1);
    } else if (!m_blockPath.isEmpty()) {
        auto *block = resolveBlock();
        block->inlineContent.append(std::move(container));
        m_inlinePath.append(block->inlineContent.size() - 1);
    }
}

/*!
    Resolves the current block path to a ContentBlock pointer.

    The returned pointer is valid only until the next QList mutation
    on any list in the path. Callers must use the pointer within a
    single expression and discard it before appending to any QList.
*/
ContentBlock *ContentBuilder::resolveBlock()
{
    Q_ASSERT(!m_blockPath.isEmpty());
    Q_ASSERT(m_blockPath[0] >= 0 && m_blockPath[0] < m_result.size());
    ContentBlock *block = &m_result[m_blockPath[0]];
    for (qsizetype i = 1; i < m_blockPath.size(); ++i) {
        Q_ASSERT(m_blockPath[i] >= 0 && m_blockPath[i] < block->children.size());
        block = &block->children[m_blockPath[i]];
    }
    return block;
}

/*!
    Resolves the current inline path to an InlineContent pointer.

    Walks the block path first (via resolveBlock()), then descends
    through the block's inlineContent and nested children lists
    using the indices in m_inlinePath.
*/
InlineContent *ContentBuilder::resolveInline()
{
    ContentBlock *block = resolveBlock();
    Q_ASSERT(!m_inlinePath.isEmpty());
    Q_ASSERT(m_inlinePath[0] >= 0 && m_inlinePath[0] < block->inlineContent.size());
    InlineContent *ic = &block->inlineContent[m_inlinePath[0]];
    for (qsizetype i = 1; i < m_inlinePath.size(); ++i) {
        Q_ASSERT(m_inlinePath[i] >= 0 && m_inlinePath[i] < ic->children.size());
        ic = &ic->children[m_inlinePath[i]];
    }
    return ic;
}

} // namespace IR

QT_END_NAMESPACE
