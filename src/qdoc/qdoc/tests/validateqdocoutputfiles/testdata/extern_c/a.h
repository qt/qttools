// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

/*!
    \headerfile a.h
    \inmodule ExternC
*/

/*!
    \fn int c_function(char *param)
    \relates a.h
    \brief C function that returns a number for \a param.
*/
extern "C" int c_function(char *param);
