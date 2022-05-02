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
#include "../../utilities/semantics/generator_handler.hpp"

#include <catch.hpp>

#include <vector>

namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE {
    namespace QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE {

        template<typename T>
        class CycleGenerator : public Catch::Generators::IGenerator<T> {
        public:
            CycleGenerator(Catch::Generators::GeneratorWrapper<T>&& generator)
                : generator{std::move(generator)},
                  cache{},
                  cache_index{0}
            {
                // REMARK: We generally handle extracting the first
                // value by using an handler, to avoid code
                // duplication and the possibility of an error.
                // In this specific case, we turn to a more "manual"
                // approach as it better models the cache-based
                // implementation, removing the need to not increment
                // cache_index the first time that next is called.
                cache.emplace_back(this->generator.get());
            }

            T const& get() const override { return cache[cache_index]; }

            bool next() override {
                if (generator.next()) {
                    cache.emplace_back(generator.get());
                    ++cache_index;
                } else {
                    cache_index = (cache_index + 1) % cache.size();
                }

                return true;
            }

        private:
            Catch::Generators::GeneratorWrapper<T> generator;

            std::vector<T> cache;
            std::size_t cache_index;
        };

    } // end QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE

    /*!
     * Returns a generator that behaves like \a generator until \a
     * generator is exhausted, repeating the same generation that \a
     * generator produced, infinitely, afterwards.
     *
     * This is generally intended to produce infinite generators from
     * finite ones.
     *
     * For example, consider a generator that produces values based on
     * another generator that it owns.
     * If the owning generator needs to produce more values that the
     * owned generator can support, it might fail at some point.
     * By cycling over the owned generator, we can extend the sequence
     * of produced values so that enough are generated, in a controlled
     * way.
     *
     * The type T should generally be copyable for this generator to
     * work.
     */
    template<typename T>
    inline Catch::Generators::GeneratorWrapper<T> cycle(Catch::Generators::GeneratorWrapper<T>&& generator) {
        return Catch::Generators::GeneratorWrapper<T>(std::unique_ptr<Catch::Generators::IGenerator<T>>(new QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE::CycleGenerator(std::move(generator))));
    }

} // end QDOC_CATCH_GENERATORS_ROOT_NAMESPACE
