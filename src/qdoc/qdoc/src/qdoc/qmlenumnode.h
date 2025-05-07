// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLENUMNODE_H
#define QMLENUMNODE_H

#include "enumnode.h"
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QmlEnumNode : public EnumNode
{
public:
    QmlEnumNode(Aggregate *parent, const QString &name) :
        EnumNode(NodeType::QmlEnum, parent, name)
    {
        setGenus(Genus::QML);
    }
};

QT_END_NAMESPACE

#endif // QMLENUMNODE_H

