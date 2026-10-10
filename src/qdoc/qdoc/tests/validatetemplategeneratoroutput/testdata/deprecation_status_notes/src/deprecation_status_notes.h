// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module DeprecationStatusNotes
    \title Deprecation Status Notes
    \brief A module documenting the deprecation status sentence cases.
*/

/*!
    \class DeprecatedThing
    \inmodule DeprecationStatusNotes

    \brief A class that was deprecated in 6.5.
    \deprecated [6.5]
*/

class DeprecatedThing
{
public:
    DeprecatedThing();
    ~DeprecatedThing();
};

/*!
    \class PendingThing
    \inmodule DeprecationStatusNotes

    \brief A class scheduled for deprecation in 6.66.
    \deprecated [6.66]

    The class remains fully functional until 6.66.
*/

class PendingThing
{
};

/*!
    \class ActiveThing
    \inmodule DeprecationStatusNotes

    \brief A class with one deprecated and one scheduled function.
*/

class ActiveThing
{
public:
    /*!
        \brief An operation that was deprecated in 6.5.
        \deprecated [6.5]
    */
    void oldOperation();

    /*!
        \brief An operation scheduled for deprecation in 6.66.
        \deprecated [6.66]
    */
    void scheduledOperation();
};

/*!
    \class LegacyThing
    \inmodule DeprecationStatusNotes

    \brief A class deprecated without a version.
    \deprecated

    The class still works, but new code should avoid it.
*/

class LegacyThing
{
public:
    /*!
        \brief An operation deprecated without a version.
        \deprecated

        The retired operation that new code should stop calling.
    */
    void oldOperation();

    /*!
        \brief An operation that is fully current.

        The current operation that new code should call instead.
    */
    void currentOperation();
};

/*!
    \namespace LegacyNotes
    \inmodule DeprecationStatusNotes

    \brief A namespace deprecated in 6.5.
    \deprecated [6.5]

    The namespace still works, but new code should avoid it.
*/

namespace LegacyNotes
{
}
