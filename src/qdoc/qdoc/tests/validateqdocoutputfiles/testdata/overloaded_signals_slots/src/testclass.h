// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTCLASS_H
#define TESTCLASS_H

#include <QtCore/QObject>
#include <QtCore/QString>

class TestClass : public QObject
{
    Q_OBJECT

public:
    explicit TestClass(QObject *parent = nullptr);

signals:
    void dataChanged(int value);
    void dataChanged(const QString &value);
    void internalStateChanged();

public slots:
    void process(int value);
    void process(const QString &value);

private Q_SLOTS:
    void _q_doProcess();
};

#endif // TESTCLASS_H
