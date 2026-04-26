// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_CATALOGENTRY_H
#define QDOC_IR_CATALOGENTRY_H

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

namespace IR {

struct CatalogEntry
{
    QString name;
    QString href;
    QString brief;
    QString since;
    bool isDeprecated { false };
};

struct CatalogEntryGroup
{
    QString label;
    QString anchorId;
    QList<CatalogEntry> entries;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_CATALOGENTRY_H
