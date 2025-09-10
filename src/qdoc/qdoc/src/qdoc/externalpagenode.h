// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef EXTERNALPAGENODE_H
#define EXTERNALPAGENODE_H

#include "pagenode.h"

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class ExternalPageNode : public PageNode
{
public:
    ExternalPageNode(Aggregate *parent, const QString &url)
        : PageNode(Node::ExternalPage, parent, url)
    {
        setUrl(url);
    }
};

QT_END_NAMESPACE

#endif // EXTERNALPAGENODE_H
