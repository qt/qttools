// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "contentblock.h"

#include <QJsonArray>
#include <QStringList>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \enum IR::InlineType
    \internal
    \brief Discriminator for inline content elements within a block.

    Inline types represent formatting and content that flows within a block
    element (paragraph, heading, etc.). They can nest — for example, Bold
    containing Text, or Link containing Code.

    \value Text Plain text content.
    \value Code Inline code span, such as a class or function name.
    \value Link Hyperlink to another page or external resource.
    \value Bold Bold-formatted text.
    \value Italic Italic-formatted text.
    \value Teletype Monospaced text, typically rendered as \c{<tt>}.
    \value Underline Underlined text.
    \value Strikethrough Struck-through text.
    \value Subscript Subscript text.
    \value Superscript Superscript text.
    \value Parameter Function parameter name.
    \value LineBreak Explicit line break within a block.
    \value Image Inline image.
    \value Keyword Index keyword anchor.
    \value Target Named anchor target for cross-references.
*/

/*!
    \enum IR::BlockType
    \internal
    \brief Discriminator for structural block elements in documentation.

    Block types represent the structural elements of documentation content:
    paragraphs, code blocks, lists, sections, and callout blocks. They nest
    to form the document structure (e.g., List containing ListItems, each
    containing Paragraphs).

    \value Paragraph A paragraph of text with inline content.
    \value CodeBlock A block of source code, optionally with a language attribute.
    \value List An ordered or unordered list containing ListItem children.
    \value ListItem A single item within a List.
    \value Section A document section containing a heading and child blocks.
    \value SectionHeading A section heading with a level attribute (1–6).
    \value Note A note callout block.
    \value Warning A warning callout block.
    \value Important An important callout block.
    \value Details A collapsible details block.
    \value Brief The brief description of a documented entity.
    \value Div A generic container block.
    \value Quotation A block quotation.
    \value Legalese A legal text block, such as a license notice.
    \value HorizontalRule A horizontal separator rule.
    \value Table A table container with TableRow and TableHeaderRow children.
    \value TableRow A data row within a table, containing TableCell children.
    \value TableHeaderRow A header row within a table, containing header cells.
    \value TableCell A cell within a table row.
    \value Raw Raw format-specific content passed through without processing.
    \value DefinitionList A definition or value list containing DefinitionTerm and DefinitionDescription pairs.
    \value DefinitionTerm The term or key in a definition list entry.
    \value DefinitionDescription The description or value in a definition list entry.
    \value ListPlaceholder A short-lived placeholder emitted by the
           content builder for \\generatelist and \\annotatedlist
           atoms. The list-expander pass replaces the placeholder with
           a Catalog subtree before rendering. A placeholder that
           reaches a template indicates the expansion pass didn't run
           and the IR isn't resolved.
    \value Catalog A populated catalog wrapper, emitted by the
           list-expander pass, containing reused Table, SectionHeading,
           List, ListItem, and Link children. Renderers dispatch on
           the block's variant attribute to select the catalog style
           such as classes index, examples index, or group members.
*/

/*!
    \struct IR::InlineContent
    \internal
    \brief Represents inline content within a documentation block.

    InlineContent is a format-agnostic representation of inline content
    such as text, code, links, and formatting. Instances nest recursively
    to represent formatting like bold text containing a link.

    Each element is either a \e leaf (has \c text, no \c children) or a
    \e container (has \c children, no \c text). This invariant is enforced
    by Q_ASSERT in debug builds. Typically, content-bearing types such as
    Text, Code, and Image are leaves, while formatting types such as Bold,
    Italic, and Link are containers whose children carry the text.

    The \c href and \c title fields are metadata that can be set on either
    leaf or container elements (e.g., Link uses \c href with children;
    Image uses \c href as a leaf).

    This is a pure value type with no dependencies on QDoc's core
    infrastructure. It belongs in QDocLib.

    \sa ContentBlock, BlockType
*/

/*!
    \struct IR::ContentBlock
    \internal
    \brief Represents a structural block element in documentation.

    ContentBlock is a format-agnostic representation of documentation
    structure. Most blocks are either \e{leaf blocks} (with \c inlineContent)
    or \e{container blocks} (with \c children). Some atom chains produce
    blocks that mix both — serialization and plainText() handle this
    gracefully.

    The \c attributes field holds type-specific metadata as a QJsonObject.
    Attribute keys use camelCase for compatibility with Inja dot notation
    in templates (e.g., \c{block.attributes.listType}). Type IDs in the
    \c type field use kebab-case (e.g., \c{"code-block"}).

    This is a pure value type with no dependencies on QDoc's core
    infrastructure. It belongs in QDocLib. Multiple renderers can read
    the same ContentBlock concurrently — the frozen IR design supports
    parallel rendering per output format.

    \sa InlineContent, InlineType
*/

// Returns the kebab-case string ID for an InlineType.
static QString inlineTypeId(InlineType type)
{
    switch (type) {
    case InlineType::Text:          return u"text"_s;
    case InlineType::Code:          return u"code"_s;
    case InlineType::Link:          return u"link"_s;
    case InlineType::Bold:          return u"bold"_s;
    case InlineType::Italic:        return u"italic"_s;
    case InlineType::Teletype:      return u"teletype"_s;
    case InlineType::Underline:     return u"underline"_s;
    case InlineType::Strikethrough: return u"strikethrough"_s;
    case InlineType::Subscript:     return u"subscript"_s;
    case InlineType::Superscript:   return u"superscript"_s;
    case InlineType::Parameter:     return u"parameter"_s;
    case InlineType::LineBreak:     return u"line-break"_s;
    case InlineType::Image:         return u"image"_s;
    case InlineType::Keyword:       return u"keyword"_s;
    case InlineType::Target:        return u"target"_s;
    }
    Q_UNREACHABLE();
}

// Returns the kebab-case string ID for a BlockType.
static QString blockTypeId(BlockType type)
{
    switch (type) {
    case BlockType::Paragraph:      return u"paragraph"_s;
    case BlockType::CodeBlock:      return u"code-block"_s;
    case BlockType::List:           return u"list"_s;
    case BlockType::ListItem:       return u"list-item"_s;
    case BlockType::Section:        return u"section"_s;
    case BlockType::SectionHeading: return u"section-heading"_s;
    case BlockType::Note:           return u"note"_s;
    case BlockType::Warning:        return u"warning"_s;
    case BlockType::Important:      return u"important"_s;
    case BlockType::Details:        return u"details"_s;
    case BlockType::Brief:          return u"brief"_s;
    case BlockType::Div:            return u"div"_s;
    case BlockType::Quotation:      return u"quotation"_s;
    case BlockType::Legalese:       return u"legalese"_s;
    case BlockType::HorizontalRule: return u"horizontal-rule"_s;
    case BlockType::Table:          return u"table"_s;
    case BlockType::TableRow:       return u"table-row"_s;
    case BlockType::TableHeaderRow: return u"table-header-row"_s;
    case BlockType::TableCell:      return u"table-cell"_s;
    case BlockType::Raw:            return u"raw"_s;
    case BlockType::DefinitionList:        return u"definition-list"_s;
    case BlockType::DefinitionTerm:        return u"definition-term"_s;
    case BlockType::DefinitionDescription: return u"definition-description"_s;
    case BlockType::ListPlaceholder:       return u"list-placeholder"_s;
    case BlockType::Catalog:               return u"catalog"_s;
    }
    Q_UNREACHABLE();
}

/*!
    Converts the InlineContent to a QJsonObject for template rendering.

    The JSON uses kebab-case type IDs matching the convention in IR::Document
    classification. Every element includes a \c text key: leaf elements use
    their text field directly, while container elements produce a flattened
    plain-text concatenation of their children. This ensures templates can
    always access \c text without checking whether the element is a leaf or
    container.

    Optional fields (\c href, \c title) are omitted when empty.
    The \c children array is omitted when empty.
*/
QJsonObject InlineContent::toJson() const
{
    Q_ASSERT(children.isEmpty() || text.isEmpty());

    QJsonObject json;
    json["type"_L1] = inlineTypeId(type);
    json["text"_L1] = plainText();

    if (!href.isEmpty())
        json["href"_L1] = href;
    if (!title.isEmpty())
        json["title"_L1] = title;

    QJsonArray childArr;
    for (const auto &child : children)
        childArr.append(child.toJson());
    json["children"_L1] = childArr;

    if (!attributes.isEmpty())
        json["attributes"_L1] = attributes;

    return json;
}

/*!
    Returns the concatenated plain text of this inline element and all its
    children, recursively.

    For leaf elements (Text, Code, Keyword, Target, Parameter, Image),
    returns the \c text field. For LineBreak, returns a newline character.
    For container elements (Bold, Italic, Link, etc.), concatenates the
    plain text of all children.
*/
QString InlineContent::plainText() const
{
    Q_ASSERT(children.isEmpty() || text.isEmpty());
    if (type == InlineType::LineBreak)
        return u"\n"_s;

    if (!children.isEmpty()) {
        QStringList parts;
        parts.reserve(children.size());
        for (const auto &child : children)
            parts.append(child.plainText());
        return parts.join(u""_s);
    }

    return text;
}

/*!
    Converts the ContentBlock to a QJsonObject for template rendering.

    The JSON uses kebab-case type IDs. A computed \c text key contains the
    concatenated plain text of all inline content or children. Empty
    collections (\c inlines, \c children, \c attributes) are omitted.
*/
QJsonObject ContentBlock::toJson() const
{
    QJsonObject json;
    json["type"_L1] = blockTypeId(type);
    json["text"_L1] = plainText();

    if (!attributes.isEmpty())
        json["attributes"_L1] = attributes;

    if (!inlineContent.isEmpty()) {
        QJsonArray arr;
        for (const auto &inline_ : inlineContent)
            arr.append(inline_.toJson());
        json["inlines"_L1] = arr;
    }

    {
        QJsonArray arr;
        for (const auto &child : children)
            arr.append(child.toJson());

        if (type == BlockType::Table)
            json["rows"_L1] = arr;
        else if (type == BlockType::TableRow || type == BlockType::TableHeaderRow)
            json["cells"_L1] = arr;
        else
            json["children"_L1] = arr;
    }

    return json;
}

/*!
    Returns the concatenated plain text of this block's content,
    recursively. Collects inline text first, then child block text.
    Blocks may have both inline content and children when the atom
    chain mixes inline and block-level elements within the same
    container.
*/
QString ContentBlock::plainText() const
{
    QStringList parts;

    if (!inlineContent.isEmpty()) {
        parts.reserve(inlineContent.size());
        for (const auto &inline_ : inlineContent)
            parts.append(inline_.plainText());
    }

    if (!children.isEmpty()) {
        QStringList childParts;
        for (const auto &child : children)
            childParts.append(child.plainText());
        if (!parts.isEmpty())
            parts.append(u"\n"_s);
        parts.append(childParts.join(u"\n"_s));
    }

    return parts.join(u""_s);
}

} // namespace IR

QT_END_NAMESPACE

