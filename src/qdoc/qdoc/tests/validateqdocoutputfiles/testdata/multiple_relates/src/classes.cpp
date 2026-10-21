// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "classes.h"

/*!
    \module MultipleRelates
    \title Multiple Relates Test
    \brief Tests related non-members with multiple \\relates commands.
*/

/*!
    \class Alpha
    \inmodule MultipleRelates
    \brief The first class.
*/

/*!
    Constructs an Alpha.
*/
Alpha::Alpha() { }

/*!
    \class Beta
    \inmodule MultipleRelates
    \brief The second class.
*/

/*!
    Constructs a Beta.
*/
Beta::Beta() { }

/*!
    \class Gamma
    \inmodule MultipleRelates
    \brief The third class.
*/

/*!
    Constructs a Gamma.
*/
Gamma::Gamma() { }

/*!
    \relates Alpha
    \relates Beta
    \relates Gamma
    \relates Beta

    Converts \a from to \a to. Returns \c true on success.
*/
bool convert(const Alpha &from, Beta &to)
{
    return true;
}

/*!
    \fn bool operator==(const Alpha &lhs, const Beta &rhs)
    \fn bool operator!=(const Alpha &lhs, const Beta &rhs)
    \relates Beta
    \relates Alpha

    Compares \a lhs and \a rhs.
*/

/*!
    \relates Gamma
    \relates Gamma

    Resets \a gamma.
*/
void reset(Gamma &gamma) { }
