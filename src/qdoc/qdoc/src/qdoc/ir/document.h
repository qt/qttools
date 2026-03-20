// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_DOCUMENT_H
#define QDOC_IR_DOCUMENT_H

#include "contentblock.h"
#include "member.h"

#include "qdoc/access.h"
#include "qdoc/genustypes.h"
#include "qdoc/status.h"

#include <QJsonObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE

namespace IR {

struct Document
{
    // Classification
    NodeType nodeType { NodeType::NoType };
    Genus genus { Genus::DontCare };
    Status status { Status::Active };
    Access access { Access::Public };

    // Identity
    QString title;              // Page title
    QString fullTitle;          // Full qualified title
    QString url;                // Output file URL (relative)
    QString since;              // Version introduced (e.g., "6.8")
    QString deprecatedSince;    // Version deprecated (e.g., "6.5")
    QString brief;              // Brief description

    // Content
    QList<ContentBlock> body;
    QJsonObject contentJson;

    // Members (for aggregate pages)
    QList<SectionIR> summarySections;

    QJsonObject toJson() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_DOCUMENT_H

