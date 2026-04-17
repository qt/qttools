// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INTERNALCLASS_H
#define INTERNALCLASS_H

#include <QtCore/QObject>
#include <QtCore/QString>

class InternalClass : public QObject
{
    Q_OBJECT

public:
    explicit InternalClass(QObject *parent = nullptr);

signals:
    void ping();

public slots:
    void pong();
};

#endif // INTERNALCLASS_H
