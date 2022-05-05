/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
