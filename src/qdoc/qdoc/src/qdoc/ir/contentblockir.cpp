// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "contentblockir.h"

#include <QJsonArray>
#include <QStringList>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

/*!
    \enum InlineType
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
    \struct InlineContentIR
    \internal
    \brief Represents inline content within a documentation block.

    InlineContentIR is a format-agnostic representation of inline content
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

/*!
    Converts the InlineContentIR to a QJsonObject for template rendering.

    The JSON uses kebab-case type IDs matching the convention in DocumentIR
    classification. Leaf elements (no children) include a \c text key with
    their text content. Container elements (with children) omit \c text to
    avoid redundancy — the text is available in their children.

    Optional fields (\c href, \c title) are omitted when empty.
    The \c children array is omitted when empty.
*/
QJsonObject InlineContentIR::toJson() const
{
    Q_ASSERT(children.isEmpty() || text.isEmpty());

    QJsonObject json;
    json["type"_L1] = inlineTypeId(type);

    if (children.isEmpty())
        json["text"_L1] = plainText();

    if (!href.isEmpty())
        json["href"_L1] = href;
    if (!title.isEmpty())
        json["title"_L1] = title;

    if (!children.isEmpty()) {
        QJsonArray arr;
        for (const auto &child : children)
            arr.append(child.toJson());
        json["children"_L1] = arr;
    }

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
QString InlineContentIR::plainText() const
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

QT_END_NAMESPACE
