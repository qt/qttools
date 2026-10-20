// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import OtherModule as OM

/*!
    \qmltype Panel
    \inqmlmodule TestModule
    \brief A QML type documented from a QML source file.

    The file imports OtherModule under the alias OM. Import-qualified
    and unqualified dot-path links; unqualified names resolve against
    the type's own module:
    \list
    \li \l OtherModule.Layout
    \li \l OtherModule.Button.text
    \li \l Layout.fillHeight
    \li \l Layout.values.integer
    \li \l OM.Button.text
    \endlist
*/
Layout {
    id: panel

    /*!
        \qmlproperty string Panel::caption
        \brief A property with an import-alias link in its documentation.

        Fully qualified control: \l OtherModule.Button.text.
        Alias-qualified member link: \l OM.Button.text.
        Enclosing-module member link: \l Layout.fillHeight.

        \sa OM.Button.text
    */
    property string caption: ""
}
