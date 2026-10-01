// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "widget.h"

/*!
    \module RelatesScope
    \title Relates Scope Module
    \brief Related non-members declared outside the global namespace.
*/

/*!
    \class Widget
    \inmodule RelatesScope
    \brief A class that documents related non-members from namespace Ns.
*/

/*!
    Constructs a widget.
*/
Widget::Widget() {}

/*!
    \namespace Ns
    \inmodule RelatesScope
    \brief A namespace whose members are documented with Widget.
*/

/*!
    \enum Ns::TimerId
    \relates Widget

    A scoped enum declared in namespace Ns.

    \value Invalid An invalid ID.
*/

/*!
    \enum Ns::Mode
    \relates Widget

    An unscoped enum declared in namespace Ns.

    \value Fast Fast mode.
    \value Slow Slow mode.
*/

/*!
    \typealias Ns::Handle
    \relates Widget

    A type alias declared in namespace Ns.
*/

/*!
    \fn Ns::Handle Ns::makeHandle(int id)
    \fn Ns::Handle Ns::makeHandle(const char *name)
    \relates Widget

    Returns a handle for \a id or \a name. These functions are declared
    in namespace Ns and share a documentation comment.
*/

/*!
    \enum GlobalId
    \relates Widget

    A scoped enum declared in the global namespace.

    \value None No ID.
*/
