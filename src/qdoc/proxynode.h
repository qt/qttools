// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROXYNODE_H
#define PROXYNODE_H

#include "aggregate.h"

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class ProxyNode : public Aggregate
{
public:
    ProxyNode(Aggregate *parent, const QString &name);
    [[nodiscard]] bool docMustBeGenerated() const override { return true; }
    [[nodiscard]] bool isRelatableType() const override { return true; }
};

QT_END_NAMESPACE

#endif // PROXYNODE_H
