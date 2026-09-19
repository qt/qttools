// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#pragma once

/*!
    \headerfile a.h
    \inmodule ExternC
    \brief Functions declared with C linkage.
*/

/*!
    \fn int c_function(char *param)
    \relates a.h
    \brief Returns a number for \a param.
*/
extern "C" int c_function(char *param);
