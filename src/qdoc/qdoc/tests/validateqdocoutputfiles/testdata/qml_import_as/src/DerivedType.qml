// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import TestModule as TM

/*!
    \qmltype DerivedType
    \inqmlmodule TestModule
    \brief A derived type that uses import as syntax.

    This type demonstrates the bug where QDoc fails to generate proper
    links for types referenced through module aliases.

    \code
    import TestModule as TM

    TM.BaseType {
        title: "Derived"
    }
    \endcode

    The above code should properly link \l{BaseType}, even when accessed
    through the \c{TM} alias.
*/

TM.BaseType {
    id: root
    title: "Derived"

    /*!
        \qmlmethod void DerivedType::callBase()
        Calls the base type's method.
    */
    function callBase() {
        doSomething() // Should link to BaseType::doSomething()
    }
}
