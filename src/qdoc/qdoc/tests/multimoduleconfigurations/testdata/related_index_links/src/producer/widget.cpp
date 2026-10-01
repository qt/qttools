// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "widget.h"

/*!
    \module UpstreamRelated
    \title Upstream Related Module
    \brief Documents a class with related non-members from a namespace and
    the global scope.
*/

/*!
    \class Widget
    \inmodule UpstreamRelated
    \brief A class that documents related non-members from namespace Ns.
*/

/*!
    Constructs a widget.
*/
Widget::Widget() {}

/*!
    \namespace Ns
    \inmodule UpstreamRelated
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
