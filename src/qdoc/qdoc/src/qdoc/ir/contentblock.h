// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_CONTENTBLOCK_H
#define QDOC_IR_CONTENTBLOCK_H

#include <QJsonObject>
#include <QList>
#include <QString>

#include <optional>

QT_BEGIN_NAMESPACE

namespace IR {

enum class InlineType : unsigned char {
    Text,
    Code,
    Link,
    Bold,
    Italic,
    Teletype,
    Underline,
    Strikethrough,
    Subscript,
    Superscript,
    Parameter,
    LineBreak,
    Image,
    Keyword,
    Target
};

enum class LinkOrigin : unsigned char {
    Auto,
    Explicit
};

enum class LinkState : unsigned char {
    Unresolved,
    Resolved,
    External,
    Broken,
    Suppressed,
    Ignored
};

struct InlineContent
{
    InlineType type { InlineType::Text };
    QString text;                          //!< Text content
    QString href;                          //!< Link target or image source
    QString title;                         //!< Link tooltip or image alt text
    QList<InlineContent> children;         //!< Nested inline content
    QJsonObject attributes;                //!< Type-specific metadata

    struct LinkData {
        LinkOrigin origin { LinkOrigin::Auto };
        LinkState state { LinkState::Unresolved };
    };
    std::optional<LinkData> link;

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] QString plainText() const;
};

enum class BlockType : unsigned char {
    Paragraph,
    CodeBlock,
    List,
    ListItem,
    Section,
    SectionHeading,
    Note,
    Warning,
    Important,
    Details,
    Brief,
    Div,
    Quotation,
    Legalese,
    HorizontalRule,
    Table,
    TableRow,
    TableHeaderRow,
    TableCell,
    Raw
};

struct ContentBlock
{
    BlockType type { BlockType::Paragraph };
    QJsonObject attributes;                //!< Type-specific metadata
    QList<InlineContent> inlineContent;    //!< Rich inline content
    QList<ContentBlock> children;          //!< Nested blocks

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] QString plainText() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_CONTENTBLOCK_H

