// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Some.Module as Foo

/*!
    \qmltype ApplicationWindow
    \inqmlmodule Some.Module
    \brief A window that provides some basic features needed for all apps.
*/

Foo.AbstractApplicationWindow {
    id: root
}
