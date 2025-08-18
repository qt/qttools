// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \page linkparentheses.html
    \title Test Page for Link Parentheses Issue

    This page tests the fix for QTBUG-69756 where parentheses in link text
    cause premature link termination.

    Test cases:

    \list
        \li \l {linkparentheses.html} {Q(Gui)Application}
        \li \l {linkparentheses.html} {Text with (parentheses) in the middle}
        \li \l {linkparentheses.html} {(Starting with parentheses)}
        \li \l {linkparentheses.html} {Ending with (parentheses)}
        \li \l {linkparentheses.html} {Multiple (sets) of (parentheses)}
        \li \l {linkparentheses.html} {Normal text without parentheses}
    \endlist

    All of the above links should be fully hyperlinked, including the text
    within parentheses.
*/

