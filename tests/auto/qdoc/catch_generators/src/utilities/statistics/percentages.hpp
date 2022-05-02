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

#include "../../namespaces.hpp"

#include <cassert>

namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE {

    /*!
     * Returns the percentage of \amount over \a total.
     *
     * \a amount needs to be greater or equal to zero and \a total
     * needs to be greater than zero.
     */
    inline double percent_of(double amount, double total) {
        assert(amount >= 0.0);
        assert(total > 0.0);

        return (amount / total) * 100.0;
    }

    /*!
     * Given the cardinality of a set, returns the percentage
     * probability that applied to every element of the set generates
     * a uniform distribution.
     */
    inline double uniform_probability(std::size_t cardinality) {
        assert(cardinality > 0);

        return (100.0 / static_cast<double>(cardinality));
    }

    /*!
     * Returns a percentage probability that is equal to \a
     * probability.
     *
     * \a probability must be in the range [0.0, 1.0]
     */
    inline double probability_to_percentage(double probability) {
        assert(probability >= 0.0);
        assert(probability <= 1.0);

        return probability * 100.0;
    }

} // end QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE
