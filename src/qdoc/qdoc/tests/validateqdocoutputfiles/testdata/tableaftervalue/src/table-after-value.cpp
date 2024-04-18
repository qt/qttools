// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include "tableaftervalue.h"

/*!
    \class TableAfterValue
    \inmodule Test
    \brief Test that the \\table command can follow a \\value command.

    \section1 Background for this test content
    According to QTBUG-115720, whenever a \\value is followed by \\table, the
    contents of the two commands end up being merged. This is likely surprising.
    The bug report suggests a workaround, which is adding \\br between \\value
    and \\table.

    \sa TableAfterValue::Reproduces
*/

/*!
    \enum TableAfterValue::Reproduces

    \value Problem

    \table
    \row
    \li This table shouldn't be mangled by the previous \\value command.
    \endtable
*/
