// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTCLASS_H
#define TESTCLASS_H

#include <QtCore/QObject>

class TestClass : public QObject
{
    Q_OBJECT

public:
    explicit TestClass(QObject *parent = nullptr);

signals:
    void valueChanged(int value);
    void valueChanged(double value);

public slots:
    void setValue(int value);
    void setValue(double value);
};

#endif // TESTCLASS_H

