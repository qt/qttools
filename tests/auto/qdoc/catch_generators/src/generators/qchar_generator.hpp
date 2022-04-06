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

#pragma once

#include "../namespaces.hpp"

#include <catch.hpp>

#include <random>

#include <QChar>

namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE {
    namespace QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE {

        class QCharGenerator : public Catch::Generators::IGenerator<QChar> {
        public:
            QCharGenerator(
                char16_t lower_bound = std::numeric_limits<char16_t>::min(),
                char16_t upper_bound = std::numeric_limits<char16_t>::max()
            ) : random_engine{std::random_device{}()},
                distribution{static_cast<unsigned int>(lower_bound), static_cast<unsigned int>(upper_bound)}
            {
                assert(lower_bound <= upper_bound);
                static_cast<void>(next());
            }

            QChar const& get() const override { return current_character; }

            bool next() override {
                current_character = QChar(static_cast<char16_t>(distribution(random_engine)));

                return true;
            }

        private:
            QChar current_character;

            std::mt19937 random_engine;
            std::uniform_int_distribution<unsigned int> distribution;
        };

    } // end QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE


    /*!
     * Returns a generator that generates elements of QChar whose
     * ucs value is in the range [\a lower_bound, \a upper_bound].
     *
     * When \a lower_bound = \a upper_bound, the generator infinitely
     * generates the same character.
     */
    inline Catch::Generators::GeneratorWrapper<QChar> character(char16_t lower_bound = std::numeric_limits<char16_t>::min(), char16_t upper_bound = std::numeric_limits<char16_t>::max()) {
        return Catch::Generators::GeneratorWrapper<QChar>(std::unique_ptr<Catch::Generators::IGenerator<QChar>>(new QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE::QCharGenerator(lower_bound, upper_bound)));
    }

} // end QDOC_CATCH_GENERATORS_ROOT_NAMESPACE
