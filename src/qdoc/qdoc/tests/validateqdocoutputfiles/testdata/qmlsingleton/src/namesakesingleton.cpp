// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "namesakesingleton.h"

/*!
    \class NamesakeObject
    \inmodule TestModule
    \brief A C++ class with QML_SINGLETON whose name equals that of an unrelated QML type.
*/

NamesakeObject::NamesakeObject(QObject *parent)
    : QObject(parent)
{
}

QString NamesakeObject::getNamesakeMessage() const
{
    return "Message from NamesakeObject with QML_SINGLETON";
}

/*!
    \class PlainObject
    \inmodule TestModule
    \brief A C++ class without QML_SINGLETON.
*/

PlainObject::PlainObject(QObject *parent)
    : QObject(parent)
{
}

QString PlainObject::getPlainMessage() const
{
    return "Message from PlainObject without QML_SINGLETON";
}
