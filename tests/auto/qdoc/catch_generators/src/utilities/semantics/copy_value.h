/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once

#include "../../namespaces.h"

#include <type_traits>

namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE {

    /*!
     * Forces \value to be copied in an expression context.
     *
     * This is used in contexts where inferences of a type that
     * requires generality might identify a reference when ownership
     * is required.
     *
     * Note that the compiler might optmize the copy away. This is a
     * non-issue as we are only interested in breaking lifetime
     * dependencies.
     */
    template<typename T>
    std::remove_reference_t<T> copy_value(T value) { return value; }

} // end QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE
