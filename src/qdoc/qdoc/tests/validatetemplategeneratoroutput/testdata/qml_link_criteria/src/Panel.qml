// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import TestModule as TM

/*!
    \qmltype Panel
    \inqmlmodule TestModule
    \brief A type using an import alias for shared QML lookup.

    \list
    \li Alias-qualified type: \l TM.Item
    \li Regular alias: \l TM.Item.name
    \li Attached alias: \l [attached] TM.Item.name
    \li Scoped attached alias: \l [QmlLinkCriteria attached] TM.Item.name
    \li Unique attached alias: \l TM.Item.tag
    \li Alias property chain: \l TM.Item.child.name
    \li Alias grouped property: \l TM.Item.values.integer
    \li Alias signature: \l {TM.Item.select(int, float)}
    \li Alias method chain: \l {TM.Item.child.select(int, float)}
    \li Alias fragment: \l {TM.Item.select(int, float)#Selection}
    \endlist

    Invalid alias references must fail:
    \list
    \li Invalid tail: \l TM.Item.name.missing
    \li Missing attached property: \l [attached] TM.Item.child
    \li Wrong signature: \l {TM.Item.select(string)}
    \endlist
*/
Item {
    /*!
        \brief A property with alias links in its documentation.

        Attached alias from a member: \l [attached] TM.Item.name.
        Method alias from a member: \l {TM.Item.select(int, float)}.

        \sa TM.Item.child.name, {TM.Item.select(int, float)}
    */
    property string caption: ""
}
