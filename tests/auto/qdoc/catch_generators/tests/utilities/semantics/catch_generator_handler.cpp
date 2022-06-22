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

#include <catch.hpp>

#include "namespaces.h"
#include "utilities/semantics/generator_handler.h"

using namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE;

TEST_CASE(
    "Calling next 0 < n times and then calling get on a GeneratorHandler wrapping a generator behaves the same as only calling next (n-1) times and then get on the generator that is wrapped",
    "[GeneratorHandler][Utilities][Semantics][Generators]"
) {
    auto n = GENERATE(take(100, random(1, 100)));
    auto generator_values = GENERATE_COPY(take(1, chunk(n, random(0, 100000))));

    auto generator_handler = handler(Catch::Generators::from_range(generator_values.begin(), generator_values.end()));
    auto generator{Catch::Generators::from_range(generator_values.begin(), generator_values.end())};

    generator_handler.next();
    for (int times{1}; times < n; ++times) {
        generator_handler.next();
        generator.next();
    }

    REQUIRE(generator_handler.get() == generator.get());
}
