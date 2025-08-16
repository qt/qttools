// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef COLLISIONSINGLETON_H
#define COLLISIONSINGLETON_H

#include <QtCore/QObject>
#include <QtQml/qqml.h>

// First class with QML_SINGLETON - this one will be properly linked
class MySingleton2 : public QObject
{
    Q_OBJECT
    QML_SINGLETON

public:
    explicit MySingleton2(QObject *parent = nullptr);
    Q_INVOKABLE QString getCollisionMessage() const;
};

// Different namespace to test collision
namespace TestNamespace {
    // Second class with same QML name but different C++ class - should NOT get singleton flag
    class MySingleton2 : public QObject
    {
        Q_OBJECT
        // Note: NO QML_SINGLETON here - this tests that we don't accidentally
        // mark the wrong class as singleton due to name collision

    public:
        explicit MySingleton2(QObject *parent = nullptr);
        Q_INVOKABLE QString getDifferentMessage() const;
    };
}

#endif // COLLISIONSINGLETON_H
