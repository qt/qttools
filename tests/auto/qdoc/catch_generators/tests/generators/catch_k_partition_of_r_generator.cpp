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

#include "namespaces.h"
#include "generators/k_partition_of_r_generator.h"

#include <catch.hpp>

#include <numeric>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;

SCENARIO("Generating a k-partition of a real number", "[Partition][Reals]") {
    GIVEN("A real number r greater or equal to zero") {
        double r = GENERATE(take(10, random(0.0, 1000000.0)));

        AND_GIVEN("An amount of desired elements k greater than zero") {
            std::size_t k = GENERATE(take(10, random(1, 100)));

            WHEN("A k-partition of r is generated") {
                auto k_partition = GENERATE_COPY(take(10, k_partition_of_r(r, k)));

                THEN("The partition contains k elements") {
                    REQUIRE(k_partition.size() == k);

                    AND_THEN("The sum of those elements is r") {
                        REQUIRE(std::accumulate(k_partition.begin(), k_partition.end(), 0.0) == Approx(r));
                    }
                }
            }
        }
    }
}

TEST_CASE("All 1-partition of r are singleton collection with r as their element", "[Partition][Reals][SpecialCase]") {
    double r = GENERATE(take(10, random(0.0, 1000000.0)));
    auto k_partition = GENERATE_COPY(take(10, k_partition_of_r(r, 1)));

    REQUIRE(k_partition.size() == 1);
    REQUIRE(k_partition.front() == r);
}
