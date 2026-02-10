// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOCUMENTIR_H
#define DOCUMENTIR_H

#include "../access.h"
#include "../genustypes.h"
#include "../status.h"

#include <QJsonObject>
#include <QString>

QT_BEGIN_NAMESPACE

struct DocumentIR
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
    QString brief;              // Brief description

    // Content
    QJsonObject contentJson;    // Content as JSON (for template rendering)

    QJsonObject toJson() const;
};

QT_END_NAMESPACE

#endif // DOCUMENTIR_H

