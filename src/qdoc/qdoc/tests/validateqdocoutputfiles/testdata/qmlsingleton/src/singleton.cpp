// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "singleton.h"

/*!
    \module TestModule
    \brief Just another test module.
*/

/*!
    \class MySingleton
    \inmodule TestModule
    \brief A C++ class for a QML singleton type.
*/

/*!
    \qmltype MySingleton
    \inqmlmodule Test
    \brief A QML singleton type using the QML_SINGLETON macro.
*/

/*!
    Constructor for MySingleton.
*/
MySingleton::MySingleton(QObject *parent)
    : QObject(parent), m_message("Hello from singleton!")
{
}

/*!
    Returns a message.
*/
QString MySingleton::getMessage() const
{
    return m_message;
}

