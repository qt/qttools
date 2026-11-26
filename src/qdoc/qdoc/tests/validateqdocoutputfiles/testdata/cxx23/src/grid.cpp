// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module Cxx23
    \title C++23 Test Module
    \brief A module for testing C++23 declarations.
*/

/*!
    \class Grid
    \inmodule Cxx23
    \brief A grid of values with a multidimensional subscript operator.

    The two-argument subscript operator is declared only when the compiler
    supports multidimensional subscripts, a C++23 feature.
*/

/*!
    \fn int Grid::operator[](int row) const

    Returns the first value in \a row.
*/

/*!
    \fn int Grid::operator[](int row, int column) const

    Returns the value at \a row and \a column.
*/
