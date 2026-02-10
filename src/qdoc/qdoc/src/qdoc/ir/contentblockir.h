// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CONTENTBLOCKIR_H
#define CONTENTBLOCKIR_H

#include <QJsonObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE

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

struct InlineContentIR
{
    InlineType type { InlineType::Text };
    QString text;                          //!< Text content
    QString href;                          //!< Link target or image source
    QString title;                         //!< Link tooltip or image alt text
    QList<InlineContentIR> children;       //!< Nested inline content

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] QString plainText() const;
};

QT_END_NAMESPACE

#endif // CONTENTBLOCKIR_H
