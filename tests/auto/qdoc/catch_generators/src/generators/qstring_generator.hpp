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
#include "qchar_generator.hpp"

#include <catch.hpp>

#include <random>

#include <QString>

namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE {
    namespace QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE {

        class QStringGenerator : public Catch::Generators::IGenerator<QString> {
        public:
            QStringGenerator(Catch::Generators::GeneratorWrapper<QChar>&& character_generator, qsizetype minimum_length, qsizetype maximum_length)
                : character_generator{std::move(character_generator)},
                random_engine{std::random_device{}()},
                length_distribution{minimum_length, maximum_length},
                current_string{}
            {
                assert(minimum_length >= 0);
                assert(maximum_length >= 0);
                assert(minimum_length <= maximum_length);

                // REMARK: [catch-generators-semantic-first-value]
                // It is important to remember that in catch, for an
                // instance of a generator that was just constructed ,
                // it should be valid to call get.
                // This means that when we depend on an externally
                // passed generator, to avoid skipping the first
                // value, we want to call get once before calling
                // next.
                // If done otherwise, we may incur into errors for
                // those cases where, for example, the generator
                // contains a single value.
                // Or, similarly, if the values of the generator are
                // finite, we will not exhaust the entire set,
                // producing possibly difficult to spot bugs or
                // improper testing.
                //
                // This means that in the constructor of a generator
                // that depends on another generator, we must perform
                // a similar operation to next, with the difference in
                // the order of the calls to get and next.
                //
                // While it is possible to abstract this away to avoid
                // the code duplication that we have here, those
                // operation have such a small scope and small reading
                // overhead that the duplication was purposefully
                // chosen to avoid complicating the code needlessly.

                qsizetype length{length_distribution(random_engine)};

                if (length > 0) this->current_string += this->character_generator.get();

                for (qsizetype length_index{1}; length_index < length; ++length_index) {
                    if (!this->character_generator.next()) Catch::throw_exception("Not enough values to initialize the first string");
                    this->current_string += this->character_generator.get();
                }
            }

            QString const& get() const override { return current_string; }

            bool next() override {
                qsizetype length{length_distribution(random_engine)};

                current_string = QString();
                for (qsizetype length_index{0}; length_index < length; ++length_index) {
                    if (!character_generator.next()) return false;

                    current_string += character_generator.get();
                }

                return true;
            }

        private:
            Catch::Generators::GeneratorWrapper<QChar> character_generator;

            std::mt19937 random_engine;
            std::uniform_int_distribution<qsizetype> length_distribution;

            QString current_string;
        };

    } // end QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE

    /*!
     * Returns a generator that generates elements of QString from
     * some amount of elements taken from \a character_generator.
     *
     * The generated strings will have a length in the range
     * [\a minimum_length, \a maximum_length].
     *
     * For compatibility with the Qt API, it is possible to provide
     * negative bounds for the length. This is, nonetheless,
     * considered an error such that the bounds should always be
     * greater or equal to zero.
     *
     * It is similarly considered an error to have minimum_length <=
     * maximum_length.
     *
     * The provided generator will generate elements until \a
     * character_generator is exhausted.
     */
    inline Catch::Generators::GeneratorWrapper<QString> string(Catch::Generators::GeneratorWrapper<QChar>&& character_generator, qsizetype minimum_length, qsizetype maximum_length) {
        return Catch::Generators::GeneratorWrapper<QString>(std::unique_ptr<Catch::Generators::IGenerator<QString>>(new QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE::QStringGenerator(std::move(character_generator), minimum_length, maximum_length)));
    }

    /*!
     * Returns an infinite generator whose elements are the empty
     * QString.
     */
    inline Catch::Generators::GeneratorWrapper<QString> empty_string() {
        return Catch::Generators::GeneratorWrapper<QString>(std::unique_ptr<Catch::Generators::IGenerator<QString>>(new QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE::QStringGenerator(character(), 0, 0)));
    }


} // end QDOC_CATCH_GENERATORS_ROOT_NAMESPACE
