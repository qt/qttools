// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOCUMENTIR_H
#define DOCUMENTIR_H

#include <QJsonObject>
#include <QString>

QT_BEGIN_NAMESPACE

class Atom;
class Node;

struct DocumentIR
{
    QString title;              // Page title
    QString fullTitle;          // Full qualified title
    QString url;                // Output file URL (relative)
    QString brief;              // Brief description
    QJsonObject contentJson;    // Content as JSON (for template rendering)

    QJsonObject toJson() const;
};

QT_END_NAMESPACE

#endif // DOCUMENTIR_H

