// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "local.h"

/*!
    \module RelatesUser
    \title Relates User Module
    \brief Documents non-members related to classes in another module.
*/

/*!
    \class Local
    \inmodule RelatesUser
    \brief A class that is the adoptive parent of non-members.
*/

/*!
    Constructs a local.
*/
Local::Local() { }

/*!
    \relates Local
    \relates Target

    Combines \a local with \a target. Returns \c true on success.
*/
bool combine(const Local &local, const Target &target)
{
    return true;
}

/*!
    \fn bool operator==(const Local &lhs, const Other &rhs)
    \fn bool operator!=(const Local &lhs, const Other &rhs)
    \relates Local
    \relates Other

    Compares \a lhs and \a rhs.
*/
