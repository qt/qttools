/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "namespaces.hpp"
#include "generators/qchar_generator.hpp"

#include <qt_catch_conversions.hpp>

#include <catch.hpp>

#include <QChar>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;

SCENARIO("Binding a generated QChar to a range", "[QChar][Bounds]") {
    GIVEN("A lower bound") {
        auto lower_bound = GENERATE(take(100, random(
            static_cast<unsigned int>(std::numeric_limits<char16_t>::min()),
            static_cast<unsigned int>(std::numeric_limits<char16_t>::max())
        )));

        AND_GIVEN("An upper bound that is greater or equal than the lower bound") {
            auto upper_bound = GENERATE_COPY(take(100, random(lower_bound, static_cast<unsigned int>(std::numeric_limits<char16_t>::max()))));

            WHEN("A QChar is generated from those bounds") {
                QChar generated_character = GENERATE_COPY(take(1, character(lower_bound, upper_bound)));

                THEN("The generated character has a unicode value in the range [lower_bound, upper_bound]") {
                    REQUIRE(generated_character.unicode() >= lower_bound);
                    REQUIRE(generated_character.unicode() <= upper_bound);
                }
            }
        }
    }
}

TEST_CASE(
    "When lower_bound and upper_bound are equal, let their value be n, the only generated character is the one with unicode value n",
    "[QChar][Bounds]"
) {
    auto bound = GENERATE(take(100, random(
        static_cast<unsigned int>(std::numeric_limits<char16_t>::min()),
        static_cast<unsigned int>(std::numeric_limits<char16_t>::max())
    )));
    auto generated_character = GENERATE_COPY(take(100, character(bound, bound)));

    REQUIRE(generated_character.unicode() == bound);
}
