// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SINGLETON_H
#define SINGLETON_H

#include <QtCore/QObject>
#include <QtQml/qqml.h>

class MySingleton : public QObject
{
    Q_OBJECT
    QML_SINGLETON

public:
    explicit MySingleton(QObject *parent = nullptr);

    Q_INVOKABLE QString getMessage() const;

private:
    QString m_message;
};

#endif // SINGLETON_H

