// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_LISTEXPANDER_H
#define QDOC_IR_LISTEXPANDER_H

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "catalogentry.h"
#include "listplaceholder.h"

#include <QtCore/qlist.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>

#include <functional>

QT_BEGIN_NAMESPACE

class Node;

namespace IR {

struct ContentBlock;

struct ListExpanderCallbacks
{
    std::function<QList<CatalogEntry>(const Node *relative,
                                      Qt::SortOrder sortOrder)>
            collectCppClasses;

    // Invoked when a placeholder's enumeration produces no entries,
    // so the driver can attribute the warning. The expander itself
    // logs nothing — keeping QDocLib free of driver-layer logging
    // categories. The callback is optional; an unset slot means the
    // empty placeholder is silently dropped.
    std::function<void(const QString &argument,
                       ListPlaceholderVariant variant)>
            onEmpty;
};

class ListExpander
{
public:
    explicit ListExpander(ListExpanderCallbacks callbacks);

    void expand(QList<ContentBlock> &blocks, const Node *relative);

private:
    ListExpanderCallbacks m_callbacks;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
#endif // QDOC_IR_LISTEXPANDER_H
