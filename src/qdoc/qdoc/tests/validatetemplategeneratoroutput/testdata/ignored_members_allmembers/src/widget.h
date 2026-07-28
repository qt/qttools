// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

/*!
    \module IgnoredMembers
    \title Ignored Members Test Module
    \brief A module for testing that ignorable members stay off the
    all-members list.
*/

/*!
    \class Widget
    \inmodule IgnoredMembers
    \brief A widget whose all-members list must skip ignorable members.

    Widget carries an ordinary documented member alongside an undocumented
    member whose name marks it as silently ignorable. The all-members page
    must list the documented member and omit the ignorable one.
*/
class Widget
{
public:
    /*!
        Repaints the widget.

        An ordinary documented member that must appear on the all-members
        page.
    */
    void repaint();

    // Undocumented, and named to trigger FunctionNode::isIgnored(). It must
    // never surface on the all-members page.
    void _q_ignorableSlot();
};
