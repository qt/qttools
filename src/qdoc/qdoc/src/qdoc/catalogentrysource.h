// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_CATALOGENTRYSOURCE_H
#define QDOC_CATALOGENTRYSOURCE_H

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "ir/catalogentry.h"
#include "inclusionpolicy.h"

#include <QtCore/qnamespace.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class HrefResolver;
class Node;
class QDocDatabase;

class CatalogEntrySource
{
public:
    CatalogEntrySource(QDocDatabase &qdb, const HrefResolver &hrefResolver,
                       InclusionPolicy policy);

    [[nodiscard]] QList<IR::CatalogEntry> collectCppClasses(
            const Node *relative, Qt::SortOrder sortOrder) const;

    [[nodiscard]] QList<IR::CatalogEntryGroup> collectExamplesGrouped(
            const Node *relative) const;

    [[nodiscard]] QList<IR::CatalogEntry> collectCompactClasses(
            const Node *relative, const QString &rootName) const;

    [[nodiscard]] QList<IR::CatalogEntry> collectGroupMembers(
            const Node *relative, const QString &groupName,
            Qt::SortOrder sortOrder) const;

private:
    QDocDatabase &m_qdb;
    const HrefResolver &m_hrefResolver;
    InclusionPolicy m_inclusionPolicy;
};

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
#endif // QDOC_CATALOGENTRYSOURCE_H
