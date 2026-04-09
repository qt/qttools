// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "contentbuilder.h"

#include "../atom.h"
#include "../tree.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

static QString stripCodeMarkerTags(const QString &markedCode)
{
    static const QRegularExpression tag(u"</?@[^>]*>"_s);
    QString t = markedCode;
    t.replace(tag, QString());
    t.replace(u"&quot;"_s, u"\""_s);
    t.replace(u"&gt;"_s, u">"_s);
    t.replace(u"&lt;"_s, u"<"_s);
    t.replace(u"&amp;"_s, u"&"_s);
    return t;
}

static QString genusToString(Genus genus)
{
    switch (genus) {
    case Genus::CPP: return u"cpp"_s;
    case Genus::QML: return u"qml"_s;
    case Genus::DOC: return u"doc"_s;
    case Genus::API: return u"api"_s;
    case Genus::DontCare: return {};
    }
    return {};
}

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
        \li Code, CodeBad, Qml -- Code blocks with language attribute.
        \li SectionLeft, SectionRight -- Section containers.
        \li SectionHeadingLeft, SectionHeadingRight -- Section headings with
            level.
        \li FormattingLeft, FormattingRight -- Bold, italic, teletype,
            underline, subscript, superscript, parameter, uicontrol, trademark,
            link, index, notranslate, span.
        \li ListLeft, ListRight -- Ordered and unordered lists.
        \li ListItemLeft, ListItemRight -- List items.
        \li ListItemNumber -- List start number metadata.
        \li NoteLeft, NoteRight -- Note admonition blocks.
        \li WarningLeft, WarningRight -- Warning admonition blocks.
        \li BriefLeft, BriefRight -- Brief exclusion (skipped by default,
            or emitted as Paragraph with BriefHandling::Include).
        \li Link, NavLink -- Explicit links with unresolved target.
        \li AutoLink, NavAutoLink -- Auto-linked type names with unresolved
            target.
        \li BR -- Line break inline.
        \li HR -- Horizontal rule block.
        \li Nop -- No-operation (skipped).
        \li BaseName -- No-operation (skipped).
        \li TableLeft, TableRight -- Table containers with style attribute.
        \li TableHeaderLeft, TableHeaderRight -- Table header rows.
        \li TableRowLeft, TableRowRight -- Table data rows.
        \li TableItemLeft, TableItemRight -- Table cells with optional
            colspan/rowspan.
        \li Image -- Block-level image (wrapping Paragraph with centerAlign).
        \li InlineImage -- Inline image within a paragraph.
        \li ImageText -- Alt text consumed by the preceding Image or
            InlineImage handler.
        \li ListTagLeft, ListTagRight -- Value list tag items (ListItem
            blocks).
        \li SinceTagLeft, SinceTagRight -- Version tag items (skipped).
        \li AnnotatedList -- Placeholder Div.
        \li GeneratedList -- Placeholder Div.
    \endlist

    Format-conditional atoms (FormatIf, FormatElse, FormatEndif) are
    skipped unconditionally. The template generator builds a
    format-agnostic IR that serves all output formats from a single
    build pass.

    The optional BriefHandling parameter controls whether brief
    content (between BriefLeft and BriefRight atoms) is included in
    the output. The default is BriefHandling::Skip, which suppresses
    brief content. BriefHandling::Include causes brief content to be
    emitted as a Paragraph block.

    ContentBuilder depends only on Atom (for reading the chain) and IR types
    (for producing output).

    \sa ContentBlock, InlineContent
*/

/*!
    Constructs a ContentBuilder.

    The \a briefHandling parameter controls whether content between
    BriefLeft and BriefRight atoms is emitted as a Paragraph block
    (BriefHandling::Include) or suppressed (BriefHandling::Skip).

    The \a headingOffset parameter shifts section heading levels to
    account for the page structure. QDoc's \\section1 maps to level 1,
    but pages already use \c{<h1>} for the title and \c{<h2>} for
    major sections. The legacy generators apply an offset derived from
    the node type; callers pass that same offset here so the IR
    produces correct heading levels without depending on \b Node.
*/
ContentBuilder::ContentBuilder(BriefHandling briefHandling, int headingOffset,
                               DiagnosticHandler diagnosticHandler)
    : m_briefHandling(briefHandling), m_headingOffset(headingOffset),
      m_diagnose(std::move(diagnosticHandler))
{
}

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
    while (!m_blockPath.isEmpty())
        closeBlock();
    m_inlinePath.clear();
    m_inlineBaseDepths.clear();
    m_inLink = false;
    m_inBrief = false;

    return m_result;
}

/*!
    Walks the full atom chain starting at \a atom, building the
    content tree. FormatIf..FormatEndif blocks are skipped
    unconditionally via skipFormatIfBlock(). Stray FormatElse and
    FormatEndif atoms outside any FormatIf context are ignored.
*/
void ContentBuilder::processAtoms(const Atom *atom)
{
    while (atom) {
        if (atom->type() == Atom::FormatIf) {
            atom = skipFormatIfBlock(atom);
            continue;
        }
        if (atom->type() == Atom::FormatElse || atom->type() == Atom::FormatEndif) {
            atom = atom->next();
            continue;
        }
        atom = dispatchAtom(atom);
        if (!atom)
            return;
        atom = atom->next();
    }
}

/*!
    Skips an entire FormatIf..FormatEndif block, including any nested
    FormatIf blocks and FormatElse branches. The scan only tracks
    FormatIf and FormatEndif for depth counting; all other atom types
    (including FormatElse) are treated as inert content and walked
    past without dispatch.

    The template generator builds a format-agnostic IR, so
    format-conditional content is unconditionally excluded.

    Returns a pointer to the atom after FormatEndif, or \nullptr if
    the chain ends before FormatEndif is found.
*/
const Atom *ContentBuilder::skipFormatIfBlock(const Atom *atom)
{
    Q_ASSERT(atom->type() == Atom::FormatIf);
    int depth = 1;
    atom = atom->next();
    while (atom && depth > 0) {
        if (atom->type() == Atom::FormatIf)
            ++depth;
        else if (atom->type() == Atom::FormatEndif)
            --depth;
        atom = atom->next();
    }
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
        if (m_briefHandling == BriefHandling::Include)
            openBlock(BlockType::Paragraph);
        return atom;
    }
    if (atom->type() == Atom::BriefRight) {
        if (m_briefHandling == BriefHandling::Include)
            closeBlock();
        m_inBrief = false;
        return atom;
    }
    if (m_inBrief && m_briefHandling != BriefHandling::Include)
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
        addLeafInline(InlineType::Code, stripCodeMarkerTags(atom->string()));
        break;

    case Atom::Code:
    case Atom::CodeBad:
    case Atom::Qml: {
        QJsonObject attrs;
        if (atom->type() == Atom::Qml) {
            attrs["language"_L1] = u"qml"_s;
        } else if (atom->type() == Atom::CodeBad) {
            attrs["language"_L1] = u"cpp"_s;
            attrs["bad"_L1] = true;
        } else if (atom->count() >= 2 && !atom->string(1).isEmpty()) {
            attrs["language"_L1] = atom->string(1);
        } else {
            attrs["language"_L1] = u"cpp"_s;
        }

        openBlock(BlockType::CodeBlock, attrs);
        addLeafInline(InlineType::Text, stripCodeMarkerTags(atom->string()));
        closeBlock();
        break;
    }

    case Atom::AutoLink:
    case Atom::NavAutoLink: {
        // href values are not author-controlled. They are produced by QDoc
        // link resolution (\l, autolinks) against the node tree, or by
        // \image path handling. They don't contain arbitrary schemes (e.g.
        // javascript:). Only link text originates from user-authored docs
        // and must be HTML-escaped in templates.
        InlineContent link;
        link.type = InlineType::Link;
        link.href = atom->string();
        link.link = InlineContent::LinkData{ LinkOrigin::Auto, LinkState::Unresolved };
        link.children.append({ InlineType::Text, atom->string(), {}, {}, {}, {}, {} });
        addInline(std::move(link));
        break;
    }

    case Atom::Link:
    case Atom::NavLink: {
        if (m_blockPath.isEmpty())
            openBlock(BlockType::Paragraph);

        m_inLink = true;

        InlineContent link;
        link.type = InlineType::Link;
        link.href = atom->string();
        link.link = InlineContent::LinkData{ LinkOrigin::Explicit, LinkState::Unresolved };

        // Extract genus and module scope from LinkAtom if available.
        if (atom->isLinkAtom()) {
            // genus() and domain() are non-const but non-mutating;
            // the same const_cast pattern is used in qdocdatabase.cpp.
            auto *mutableAtom = const_cast<Atom *>(atom);
            Genus genus = mutableAtom->genus();
            QString genusStr = genusToString(genus);
            if (!genusStr.isEmpty())
                link.attributes["linkGenus"_L1] = genusStr;
            if (Tree *domain = mutableAtom->domain())
                link.attributes["linkModule"_L1] = domain->physicalModuleName();
        }

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
        attrs["level"_L1] = atom->string().toInt() + m_headingOffset;
        openBlock(BlockType::SectionHeading, attrs);
        break;
    }

    case Atom::SectionHeadingRight:
        closeBlock();
        break;

    case Atom::ListLeft: {
        QJsonObject attrs;
        const QString &listType = atom->string();
        attrs["listType"_L1] = listType;
        if (listType == ATOM_LIST_TAG || listType == ATOM_LIST_VALUE)
            openBlock(BlockType::DefinitionList, attrs);
        else
            openBlock(BlockType::List, attrs);
        break;
    }

    case Atom::ListRight:
        closeBlock();
        break;

    case Atom::ListItemLeft: {
        ContentBlock *parent = resolveBlock();
        if (parent->type == BlockType::DefinitionList)
            openBlock(BlockType::DefinitionDescription);
        else
            openBlock(BlockType::ListItem);
        break;
    }

    case Atom::ListItemRight:
        closeBlock();
        break;

    case Atom::ListItemNumber:
        // Start-number metadata is not yet represented in the IR.
        break;

    case Atom::NoteLeft:
        openBlock(BlockType::Note);
        break;

    case Atom::NoteRight:
        closeBlock();
        break;

    case Atom::WarningLeft:
        openBlock(BlockType::Warning);
        break;

    case Atom::WarningRight:
        closeBlock();
        break;

    case Atom::BR:
        addLeafInline(InlineType::LineBreak, {});
        break;

    case Atom::HR:
        openBlock(BlockType::HorizontalRule);
        closeBlock();
        break;

    case Atom::AnnotatedList: {
        QJsonObject attrs;
        attrs["annotatedList"_L1] = atom->string();
        openBlock(BlockType::Div, attrs);
        closeBlock();
        break;
    }

    case Atom::GeneratedList: {
        QJsonObject attrs;
        attrs["generatedList"_L1] = atom->string();
        openBlock(BlockType::Div, attrs);
        closeBlock();
        break;
    }

    case Atom::TableLeft: {
        QJsonObject attrs;
        QString tableStyle = u"generic"_s;
        QString width;

        for (int i = 0; i < atom->count(); ++i) {
            const QString &arg = atom->string(i);
            if (arg == "borderless"_L1)
                tableStyle = arg;
            else if (arg.contains('%'_L1))
                width = arg;
        }

        // Handle "100 %" (space before percent) — the percent arrives
        // as a separate atom argument, reconstruct the width value.
        if (width == "%"_L1) {
            bool ok = false;
            int pct = atom->string(0).toInt(&ok);
            width = ok ? QString::number(pct) + '%'_L1 : QString();
        }

        attrs["style"_L1] = tableStyle;
        if (!width.isEmpty())
            attrs["width"_L1] = width;
        openBlock(BlockType::Table, attrs);
        break;
    }

    case Atom::TableRight:
        closeBlock();
        break;

    case Atom::TableHeaderLeft:
        openBlock(BlockType::TableHeaderRow);
        break;

    case Atom::TableHeaderRight:
        closeBlock();
        break;

    case Atom::TableRowLeft:
        openBlock(BlockType::TableRow);
        break;

    case Atom::TableRowRight:
        closeBlock();
        break;

    case Atom::TableItemLeft: {
        QJsonObject attrs;
        const QString &spec = atom->string();
        if (!spec.isEmpty()) {
            const auto parts = QStringView{spec}.split(u',');
            if (parts.size() >= 2) {
                int colspan = qMax(1, parts[0].toInt());
                int rowspan = qMax(1, parts[1].toInt());
                if (colspan > 1)
                    attrs["colspan"_L1] = colspan;
                if (rowspan > 1)
                    attrs["rowspan"_L1] = rowspan;
            }
        }
        openBlock(BlockType::TableCell, attrs);
        break;
    }

    case Atom::TableItemRight:
        closeBlock();
        break;

    case Atom::ListTagLeft:
        openBlock(BlockType::DefinitionTerm);
        break;

    case Atom::ListTagRight:
        closeBlock();
        break;

    case Atom::Image: {
        QJsonObject attrs;
        attrs["class"_L1] = u"centerAlign"_s;
        openBlock(BlockType::Paragraph, attrs);

        InlineContent img;
        img.type = InlineType::Image;
        img.href = atom->string();
        if (atom->next() && atom->next()->type() == Atom::ImageText)
            img.title = atom->next()->string();
        addInline(std::move(img));

        closeBlock();

        if (atom->next() && atom->next()->type() == Atom::ImageText)
            return atom->next();
        break;
    }

    case Atom::InlineImage: {
        if (Q_UNLIKELY(m_blockPath.isEmpty()))
            break;

        InlineContent img;
        img.type = InlineType::Image;
        img.href = atom->string();
        if (atom->next() && atom->next()->type() == Atom::ImageText)
            img.title = atom->next()->string();
        addInline(std::move(img));

        if (atom->next() && atom->next()->type() == Atom::ImageText)
            return atom->next();
        break;
    }

    case Atom::ImageText:
        break;

    case Atom::SinceTagLeft:
    case Atom::SinceTagRight:
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
            m_inlinePath.clear();
            m_blockPath.clear();
            m_inlineBaseDepths.clear();
            m_inLink = false;
            return;
        }
        const qsizetype expectedDepth = m_inlineBaseDepths.last();
        if (m_inLink && m_inlinePath.size() > expectedDepth)
            m_inLink = false;
        m_inlinePath.resize(expectedDepth);
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
        if (m_diagnose)
            m_diagnose(QtWarningMsg, u"Dropping inline content outside any block"_s);
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
