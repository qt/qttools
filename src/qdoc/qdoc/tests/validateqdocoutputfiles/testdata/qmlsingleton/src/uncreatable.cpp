// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "uncreatable.h"

/*!
    \class MyBase
    \inmodule TestModule
    \brief A C++ class for a QML uncreatable type.
*/

/*!
    \qmltype Base
    \nativetype MyBase
    \inqmlmodule Test
    \brief A QML uncreatable type using the QML_UNCREATABLE macro.
*/

/*!
    Constructor for MyBase using \a parent.
*/
MyBase::MyBase(QObject *parent) : QObject(parent) { }
