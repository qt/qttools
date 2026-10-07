// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/*!
    \module MemberLinks
    \title Member Links
    \brief Tests links in member documentation that also match global keywords.
*/

/*!
    \class Base
    \inmodule MemberLinks
    \brief A base class.
*/

/*!
    \fn int Base::color() const

    Returns the color.
*/

/*!
    \class Derived
    \inmodule MemberLinks
    \brief A class derived from Base.

    The class body links to \l sibling and the inherited \l color.
*/

// Use the module title as the link target: its name also matches an enum value.
/*!
    \enum Derived::ReportMode

    \value MemberLinks Reports through the \l{Member Links}{MemberLinks} module.
    \value Warning Reports a warning.
*/

/*!
    \fn void Derived::paint()

    Paints with \l color, then calls \l sibling. Unrelated to \l elsewhere.

    \sa sibling
*/

/*!
    \fn void Derived::sibling()

    A sibling of paint().
*/

/*!
    \fn void Derived::fill()
    \fn void Derived::stroke()

    Shared documentation that links to \l sibling.
*/
