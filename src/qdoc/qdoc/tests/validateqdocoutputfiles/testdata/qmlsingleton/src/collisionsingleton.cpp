// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "collisionsingleton.h"

/*!
    \class MySingleton2
    \brief A C++ class with QML_SINGLETON to test name collision handling.

    This class has QML_SINGLETON and should be marked as singleton when
    properly linked via \nativetype.
*/

MySingleton2::MySingleton2(QObject *parent)
    : QObject(parent)
{
}

QString MySingleton2::getCollisionMessage() const
{
    return "Message from MySingleton2 with QML_SINGLETON";
}

/*!
    \class TestNamespace::MySingleton2
    \brief A different C++ class with same QML name but no QML_SINGLETON.

    This class should NOT be marked as singleton, even though it has the
    same name as the QML type. This tests that our detection doesn't
    accidentally mark wrong classes due to name collision.
*/

TestNamespace::MySingleton2::MySingleton2(QObject *parent)
    : QObject(parent)
{
}

QString TestNamespace::MySingleton2::getDifferentMessage() const
{
    return "Message from TestNamespace::MySingleton2 WITHOUT QML_SINGLETON";
}
