// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_PAGEMETADATA_H
#define QDOC_IR_PAGEMETADATA_H

#include "contentblock.h"

#include "qdoc/access.h"
#include "qdoc/genustypes.h"
#include "qdoc/status.h"

#include <QtCore/QList>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

namespace IR {

struct PageMetadata
{
    NodeType nodeType { NodeType::NoType };
    Genus genus { Genus::DontCare };
    Status status { Status::Active };
    Access access { Access::Public };

    QString title;
    QString fullTitle;
    QString url;
    QString since;
    QString deprecatedSince;
    QString brief;

    QList<ContentBlock> body;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_PAGEMETADATA_H
