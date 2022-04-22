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

#include "../namespaces.hpp"
#include "qchar_generator.hpp"
#include "qstring_generator.hpp"

#include <catch.hpp>

#include <random>

#include <QChar>
#include <QString>

namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE {


    struct PathGeneratorConfiguration {
        double multi_device_path_probability{0.5};
        double absolute_path_probability{0.5};
        double directory_path_probability{0.5};
        double has_trailing_separator_probability{0.5};
        std::size_t minimum_components_amount{1};
        std::size_t maximum_components_amount{10};

        PathGeneratorConfiguration& set_multi_device_path_probability(double amount) {
            multi_device_path_probability = amount;
            return *this;
        }

        PathGeneratorConfiguration& set_absolute_path_probability(double amount) {
            absolute_path_probability = amount;
            return *this;
        }

        PathGeneratorConfiguration& set_directory_path_probability(double amount) {
            directory_path_probability = amount;
            return *this;
        }

        PathGeneratorConfiguration& set_has_trailing_separator_probability(double amount) {
            has_trailing_separator_probability = amount;
            return *this;
        }

        PathGeneratorConfiguration& set_minimum_components_amount(std::size_t amount) {
            minimum_components_amount = amount;
            return *this;
        }

        PathGeneratorConfiguration& set_maximum_components_amount(std::size_t amount) {
            maximum_components_amount = amount;
            return *this;
        }
    };

    /*!
     * \class PathGeneratorConfiguration
     * \brief Defines some parameters to customize the generation of
     * paths by a PathGenerator.
     */

    /*!
     * \variable PathGeneratiorConfiguration::multi_device_path_probability
     *
     * Every path produced by a PathGenerator configured with a
     * mutli_device_path_probability of n has a probability of n to be
     * \e {Multi-Device} and a probability of 1.0 - n to not be \a
     * {Multi-Device}.
     *
     * multi_device_path_probability should be a value in the range [0.0,
     * 1.0].
     */

    /*!
     * \variable PathGeneratiorConfiguration::absolute_path_probability
     *
     * Every path produced by a PathGenerator configured with an
     * absolute_path_probability of n has a probability of n to be \e
     * {Absolute} and a probability of 1.0 - n to be \e {Relative}.
     *
     * absolute_path_probability should be a value in the range [0.0,
     * 1.0].
     */

    /*!
     * \variable PathGeneratiorConfiguration::directory_path_probability
     *
     * Every path produced by a PathGenerator configured with a
     * directory_path_probability of n has a probability of n to be \e
     * {To a Directory} and a probability of 1.0 - n to be \e {To a
     * File}.
     *
     * directory_path_probability should be a value in the range [0.0,
     * 1.0].
     */

    /*!
     * \variable PathGeneratiorConfiguration::has_trailing_separator_probability
     *
     * Every path produced by a PathGenerator configured with an
     * has_trailing_separator_probability of n has a probability of n
     * to \e {Have a Trailing Separator} and a probability of 1.0 - n
     * to not \e {Have a Trailing Separator}, when this is applicable.
     *
     * has_trailing_separator_probability should be a value in the
     * range [0.0, 1.0].
     */

    /*!
     * \variable PathGeneratiorConfiguration::minimum_components_amount
     *
     * Every path produced by a PathGenerator configured with a
     * minimum_components_amount of n will be the concatenation of at
     * least n non \e {device}, non \e {root}, non \e {separator}
     * components.
     *
     * minimum_components_amount should be greater than zero and less
     * than maximum_components_amount.
     */

    /*!
     * \variable PathGeneratiorConfiguration::maximum_components_amount
     *
     * Every path produced by a PathGenerator configured with a
     * maximum_components_amount of n will be the concatenation of at
     * most n non \e {device}, non \e {root}, non \e {separator} components.
     *
     * maximum_components_amount should be greater than or equal to
     * minimum_components_amount.
     */


    namespace QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE {

        class PathGenerator : public Catch::Generators::IGenerator<QString> {
        public:
            PathGenerator(
                Catch::Generators::GeneratorWrapper<QString>&& device_component_generator,
                Catch::Generators::GeneratorWrapper<QString>&& root_component_generator,
                Catch::Generators::GeneratorWrapper<QString>&& directory_component_generator,
                Catch::Generators::GeneratorWrapper<QString>&& filename_component_generator,
                Catch::Generators::GeneratorWrapper<QString>&& separator_component_generator,
                PathGeneratorConfiguration configuration = PathGeneratorConfiguration{}
            ) : device_component_generator{std::move(device_component_generator)},
                root_component_generator{std::move(root_component_generator)},
                directory_component_generator{std::move(directory_component_generator)},
                filename_component_generator{std::move(filename_component_generator)},
                separator_component_generator{std::move(separator_component_generator)},
                random_engine{std::random_device{}()},
                components_amount_distribution{configuration.minimum_components_amount, configuration.maximum_components_amount},
                is_multi_device_distribution{configuration.multi_device_path_probability},
                is_absolute_path_distribution{configuration.absolute_path_probability},
                is_directory_path_distribution{configuration.directory_path_probability},
                has_trailing_separator{configuration.has_trailing_separator_probability},
                current_path{}
            {
                assert(configuration.minimum_components_amount > 0);
                assert(configuration.minimum_components_amount <= configuration.maximum_components_amount);

                // REMARK: [catch-generators-semantic-first-value]
                {
                    std::size_t components_amount{components_amount_distribution(random_engine)};

                    // REMARK: As per our specification of a path, we
                    // do not count device components, and separators,
                    // when considering the amount of components in a
                    // path.
                    // This is a tradeoff that is not necessarily
                    // precise.
                    // Counting those kinds of components, on one
                    // hand, would allow a device component to stands
                    // on its own as a path, for example "C:", which
                    // might actually be correct in some path format.
                    // On the other hand, counting those kinds of
                    // components makes the construction of paths for
                    // our model much more complex with regards, for
                    // example, to the amount of component.
                    //
                    // Counting device components, since they can
                    // appear both in relative and absolute paths,
                    // makes the minimum amount of components
                    // different for different kinds of paths.
                    //
                    // Since absolute paths always require a root
                    // component, the minimum amount of components for
                    // a multi-device absolute path is 2.
                    //
                    // But an absolute path that is not multi-device
                    // would only require one minimum component.
                    //
                    // Similarly, problems arise with the existence of
                    // Windows' relative multi-device path, which
                    // require a leading separator component after a
                    // device component.
                    //
                    // This problem mostly comes from our model
                    // simplifying the definition of paths quite a bit
                    // into binary-forms.
                    // This simplifies the code and its structure,
                    // sacrificing some precision.
                    // The lost precision is almost none for POSIX
                    // based paths, but is graver for DOS paths, since
                    // they have a more complex specification.
                    //
                    // Currently, we expect that the paths that QDoc
                    // will encounter will mostly be in POSIX-like
                    // forms, even on Windows, and aim to support
                    // that, such that the simplification of code is
                    // considered a better tradeoff compared to the
                    // loss of precision.
                    //
                    // If this changes, the model should be changed to
                    // pursue a Windows-first modeling, moving the
                    // categorization of paths from the current binary
                    // model to the absolute, drive-relative and
                    // relative triptych that Windows uses.
                    // This more complex model should be able to
                    // completely describe posix paths too, making it
                    // a superior choice as long as the complexity is
                    // warranted.
                    //
                    // Do note that the model similarly can become
                    // inconsistent when used to generate format of
                    // paths such as the one used in some resource
                    // systems.
                    // Those are considered out-of-scope for our needs
                    // and were not taken into account when developing
                    // this generator.
                    if (is_multi_device_distribution(random_engine)) current_path += this->device_component_generator.get();

                    // REMARK: Similarly to not counting other form of
                    // components, we do not count root components
                    // towards the amounts of components that the path
                    // has to simplify the code.
                    // To support the "special" root path on, for
                    // example, posix systems, we require a more
                    // complex branching logic that changes based on
                    // the path being absolute or not.
                    //
                    // We don't expect root to be a particularly
                    // useful path for QDoc purposes and expect to not
                    // have to consider it for our tests.
                    // If consideration for it become required, it is
                    // possible to test it directly in the affected
                    // systemss as a special case.
                    //
                    // If most systems are affected by the handling of
                    // a root path, then the model should be slightly
                    // changed to accommodate its generation.
                    if (is_absolute_path_distribution(random_engine)) current_path += this->root_component_generator.get();

                    std::size_t prefix_components_amount{std::max(std::size_t{1}, components_amount) - 1};

                    if (prefix_components_amount > 0) {
                        current_path += this->directory_component_generator.get() + this->separator_component_generator.get();
                        --prefix_components_amount;
                    }


                    while (prefix_components_amount > 0) {
                        if (!this->directory_component_generator.next() || !this->separator_component_generator.next())
                            Catch::throw_exception("Not enough values to initialize the first string");

                        current_path += this->directory_component_generator.get() + this->separator_component_generator.get();
                        --prefix_components_amount;
                    }


                    if (is_directory_path_distribution(random_engine)) {
                        current_path += this->directory_component_generator.get();

                        if (has_trailing_separator(random_engine)) {
                            current_path += this->separator_component_generator.get();
                        }
                    } else {
                        current_path += this->filename_component_generator.get();
                    }
                }
            }

            QString const& get() const override { return current_path; }

            bool next() override {
                std::size_t components_amount{components_amount_distribution(random_engine)};

                current_path = "";

                if (is_multi_device_distribution(random_engine)) {
                    if (!device_component_generator.next()) return false;
                    current_path += device_component_generator.get();
                }

                if (is_absolute_path_distribution(random_engine)) {
                    if (!root_component_generator.next()) return false;

                    current_path += root_component_generator.get();
                }

                std::size_t prefix_components_amount{std::max(std::size_t{1}, components_amount) - 1};
                while (prefix_components_amount > 0) {
                    if (!directory_component_generator.next()) return false;
                    if (!separator_component_generator.next()) return false;

                    current_path += directory_component_generator.get() + separator_component_generator.get();
                    --prefix_components_amount;
                }

                if (is_directory_path_distribution(random_engine)) {
                    if (!directory_component_generator.next()) return false;
                    current_path += directory_component_generator.get();

                    if (has_trailing_separator(random_engine)) {
                        if (!separator_component_generator.next()) return false;
                        current_path += separator_component_generator.get();
                    }
                } else {
                    if (!filename_component_generator.next()) return false;
                    current_path += filename_component_generator.get();
                }

                return true;
            }

        private:
            Catch::Generators::GeneratorWrapper<QString> device_component_generator;
            Catch::Generators::GeneratorWrapper<QString> root_component_generator;
            Catch::Generators::GeneratorWrapper<QString> directory_component_generator;
            Catch::Generators::GeneratorWrapper<QString> filename_component_generator;
            Catch::Generators::GeneratorWrapper<QString> separator_component_generator;

            std::mt19937 random_engine;
            std::uniform_int_distribution<std::size_t> components_amount_distribution;
            std::bernoulli_distribution is_multi_device_distribution;
            std::bernoulli_distribution is_absolute_path_distribution;
            std::bernoulli_distribution is_directory_path_distribution;
            std::bernoulli_distribution has_trailing_separator;

            QString current_path;
        };

    } // end QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE

/*!
    * Returns a generator that produces QStrings that represent a
    * path in a filesystem.
    *
    * A path is formed by the following components, loosely based
    * on the abstraction that is used by std::filesystem::path:
    *
    * \list
    * \li \b {device}:
    *     Represents the device on the filesystem that
    *     the path should be considered in terms of.
    *     This is an optional components that is sometimes
    *     present on multi-device systems, such as Windows, to
    *     distinguish which device the path refers to.
    *     When present, it always appears before any other
    *     component.
    * \li \b {root}:
    *     A special sequence that marks the path as absolute.
    *     This is an optional component that is present, always,
    *     in absolute paths.
    * \li \b {directory}:
    *     A component that represents a directory on the
    *     filesystem that the path "passes-trough".
    *     Zero or more of this components can be present in the
    *     path.
    *     A path pointing to a directory on the filesystem that
    *     is not \e {root} always ends with a component of this
    *     type.
    * \li \b {filename}:
    *     A component that represents a file on the
    *     filesystem.
    *     When this component is present, it is present only once
    *     and always as the last component of the path.
    *     A path that has such a component is a path that points
    *     to a file on the filesystem.
    *     For some path formats, there is no difference in the
    *     format of a \e {filename} and a \e {directory}.
    * \li \b {separator}:
    *     A component that is interleaved between other types of
    *     components to separate them so that they are
    *     recognizable.
    *     A path that points to a directory on the filesystem may
    *     sometimes have a \e {separator} at the end, after the
    *     ending \e {directory} component.
    * \endlist
    *
    * Each component is representable as a string and a path is a
    * concatenation of the string representation of some
    * components, with the following rules:
    *
    * \list
    * \li There is at most one \e {device} component.
    * \li If a \e {device} component is present it always
    * precedes all other components.
    * \li There is at most one \e {root} component.
    * \li If a \e {root} component is present it:
    *     \list
    *     \li Succeeds the \e {device} component if it is present.
    *     \li Precedes every other components if the \e {device}
    *     component is not present.
    *     \endlist
    * \li There are zero or more \e {directory} component.
    * \li There is at most one \e {filename} component.
    * \li If a \e {filename} component is present it always
    * succeeds all other components.
    * \li Between any two successive \e {directory} components
    * there is a \e {separator} component.
    * \li Between each successive \e {directory} and \e
    * {filename} component there is a \e {separator} component.
    * \li If the last component is a \e {directory} component it
    * can be optionally followed by a \e {separator} component.
    * \li At least one component that is not a \e {device}, a \e
    * {root} or \e {separator} component is present.
    * \endlist
    *
    * For example, if "C:" is a \e {device} component, "\" is a
    * \e {root} component, \" is a \e {separator} component,
    * "directory" is a \e {directory} component and "filename" is
    * a \e {filename} component, the following are all paths:
    *
    * C:\directory", "C:\directory\directory", "C:filename",
    * "directory\directory\", "\directory\filename", "filename".
    *
    * While the following aren't:
    *
    * "C:", "C:\", "directory\C:", "foo", "C:filename\",
    * "filename\directory\filename", "filename\filename",
    * "directorydirectory"."
    *
    * The format of different components type can be the same.
    * For example, the \e {root} and \e {separator} component in
    * the above example.
    * For the purpose of generation, we do not care about the
    * format itself and consider a component of a certain type
    * depending only on how it is generated/where it is generated
    * from.
    *
    * For example, if every component is formatted as the string
    * "a", the string "aaa" could be a generated path.
    * By the string alone, it is not possible to simply discern
    * which components form it, but it would be possible to
    * generate it if the first "a" is a \a {device} component,
    * the second "a" is a \e {root} component and the third "a"
    * is a \e {directory} or \e {filename} component.
    *
    * A path, is further said to have some properties, pairs of
    * which are exclusive to each other.
    *
    * A path is said to be:
    *
    * \list
    * \li \b {Multi-Device}:
    *     When it contains a \e {device} component.
    * \li \b {Absolute}:
    *     When it contains a \e {root} component.
    *     If the path is \e {Absolute} it is not \e {Relative}.
    * \li \b {Relative}:
    *     When it does not contain a \e {root} component.
    *     If the path is \e {Relative} it is not \e {Absolute}.
    * \li \b {To a Directory}:
    *     When its last component is a \e {directory} component
    *     or a \e {directory} component followed by a \e
    *     {separator} component.
    *     If the path is \e {To a Directory} it is not \e {To a
    *     File}.
    * \li \b {To a File}:
    *     When its last component is a \e {filename}.
    *     If the path is \e {To a File} it is not \e {To a
    *     Directory}.
    * \endlist
    *
    * All path are \e {Relative/Absolute}, \e {To a
    * Directory/To a File} and \e {Multi-Device} or not.
    *
    * Furthermore, a path that is \e {To a Directory} and whose
    * last component is a \e {separator} component is said to \e
    * {Have a Trailing Separator}.
    */
    inline Catch::Generators::GeneratorWrapper<QString> path(
        Catch::Generators::GeneratorWrapper<QString>&& device_generator,
        Catch::Generators::GeneratorWrapper<QString>&& root_component_generator,
        Catch::Generators::GeneratorWrapper<QString>&& directory_generator,
        Catch::Generators::GeneratorWrapper<QString>&& separator_generator,
        Catch::Generators::GeneratorWrapper<QString>&& filename_generator,
        PathGeneratorConfiguration configuration = PathGeneratorConfiguration{}
    ) {
        return Catch::Generators::GeneratorWrapper<QString>(
            std::unique_ptr<Catch::Generators::IGenerator<QString>>(
                new QDOC_CATCH_GENERATORS_PRIVATE_NAMESPACE::PathGenerator(std::move(device_generator), std::move(root_component_generator), std::move(directory_generator), std::move(separator_generator), std::move(filename_generator), configuration)
            )
        );
    }

} // end QDOC_CATCH_GENERATORS_ROOT_NAMESPACE
