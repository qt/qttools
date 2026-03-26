// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_SIGNATURESPAN_H
#define QDOC_IR_SIGNATURESPAN_H

#include <QJsonObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE

namespace IR {

enum class SpanRole : unsigned char {
    Text,
    Type,
    Name,
    Parameter,
    Operator,
    Extra,
    TemplateDecl,
    Link,
    ExternalRef
};

struct SignatureSpan
{
    SpanRole role { SpanRole::Text };
    QString text;
    QString href;
    QList<SignatureSpan> children;

    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] QString plainText() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_SIGNATURESPAN_H
