// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UNCREATABLE_H
#define UNCREATABLE_H

#include <QtCore/QObject>
#include <QtQml/qqml.h>

class MyBase : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Base)
    QML_UNCREATABLE("An abstract base type.")

public:
    explicit MyBase(QObject *parent = nullptr);
};

#endif // UNCREATABLE_H
