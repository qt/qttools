// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_generators/namespaces.h>
#include <catch_generators/generators/qchar_generator.h>
#include <catch_generators/generators/qstring_generator.h>
#include <catch_generators/generators/path_generator.h>
#include <catch_generators/generators/combinators/cycle_generator.h>
#include <catch_generators/utilities/statistics/percentages.h>
#include <catch_generators/utilities/statistics/distribution.h>
#include <catch_generators/utilities/semantics/copy_value.h>

#include <catch_conversions/qt_catch_conversions.h>

#include <catch/catch.hpp>

#include <QString>
#include <QStringList>
#include <QRegularExpression>

using namespace QDOC_CATCH_GENERATORS_ROOT_NAMESPACE;
using namespace QDOC_CATCH_GENERATORS_UTILITIES_ABSOLUTE_NAMESPACE;

using namespace Qt::StringLiterals;

TEST_CASE("A path generated with a multi_device_path_probability of 1.0 always contains a device component.", "[Path][Content][SpecialCase]") {
    QString device_component_value{"C:"};
    auto path_generator = path(
        Catch::Generators::value(copy_value(device_component_value)),
        empty_string(),
        empty_string(),
        empty_string(),
        empty_string(),
        PathGeneratorConfiguration{}.set_multi_device_path_probability(1.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(generated_path.contains(device_component_value));
}

TEST_CASE("A path generated with a multi_device_path_probability of 0.0 never contains a device component.", "[Path][Content][SpecialCase]") {
    QString device_component_value{"C:"};
    auto path_generator = path(
        Catch::Generators::value(copy_value(device_component_value)),
        empty_string(),
        empty_string(),
        empty_string(),
        empty_string(),
        PathGeneratorConfiguration{}.set_multi_device_path_probability(0.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(!generated_path.contains(device_component_value));
}

TEST_CASE("A path generated with an absolute_path_probability of 1.0 always contains a root component.", "[Path][Content][SpecialCase]") {
    QString root_component_value{"\\"};
    auto path_generator = path(
        empty_string(),
        Catch::Generators::value(copy_value(root_component_value)),
        empty_string(),
        empty_string(),
        empty_string(),
        PathGeneratorConfiguration{}.set_absolute_path_probability(1.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(generated_path.contains(root_component_value));
}

TEST_CASE("A path generated with an absolute_path_probability of 0.0 never contains a root component.", "[Path][Content][SpecialCase]") {
    QString root_component_value{"\\"};
    auto path_generator = path(
        empty_string(),
        Catch::Generators::value(copy_value(root_component_value)),
        empty_string(),
        empty_string(),
        empty_string(),
        PathGeneratorConfiguration{}.set_absolute_path_probability(0.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(!generated_path.contains(root_component_value));
}

TEST_CASE("A path generated with a directory_path_probability of 1.0 always ends with a root, directory or directory followed by separator component.", "[Path][Content][SpecialCase]") {
    QString root_component_value{"root"};
    QString directory_component_value{"dir"};
    QString separator_component_value{"sep"};

    auto path_generator = path(
        cycle(Catch::Generators::value(QString("device"))),
        cycle(Catch::Generators::value(copy_value(root_component_value))),
        cycle(Catch::Generators::value(copy_value(directory_component_value))),
        cycle(Catch::Generators::value(QString("filename"))),
        cycle(Catch::Generators::value(copy_value(separator_component_value))),
        PathGeneratorConfiguration{}.set_directory_path_probability(1.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE((
        generated_path.endsWith(root_component_value) ||
        generated_path.endsWith(directory_component_value) ||
        generated_path.endsWith(directory_component_value + separator_component_value)
    ));
}

TEST_CASE("A path generated with a directory_path_probability of 0.0 always ends with a filename component.", "[Path][Content][SpecialCase]") {
    QString filename_component_value{"file"};

    auto path_generator = path(
        cycle(Catch::Generators::value(QString("device"))),
        cycle(Catch::Generators::value(QString("root"))),
        cycle(Catch::Generators::value(QString("dir"))),
        cycle(Catch::Generators::value(copy_value(filename_component_value))),
        cycle(Catch::Generators::value(QString("sep"))),
        PathGeneratorConfiguration{}.set_directory_path_probability(0.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(generated_path.endsWith(filename_component_value));
}

TEST_CASE("A directory path generated with a has_trailing_separator_probability of 1.0 always ends with a separator component.", "[Path][Content][SpecialCase]") {
    QString separator_component_value{"sep"};

    auto path_generator = path(
        cycle(Catch::Generators::value(QString("device"))),
        cycle(Catch::Generators::value(QString("root"))),
        cycle(Catch::Generators::value(QString("directory"))),
        cycle(Catch::Generators::value(QString("filename"))),
        cycle(Catch::Generators::value(copy_value(separator_component_value))),
        PathGeneratorConfiguration{}.set_directory_path_probability(1.0).set_has_trailing_separator_probability(1.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(generated_path.endsWith(separator_component_value));
}

TEST_CASE("A directory path generated with a has_trailing_separator_probability of 0.0 never ends with a separator component.", "[Path][Content][SpecialCase]") {
    QString separator_component_value{"sep"};

    auto path_generator = path(
        cycle(Catch::Generators::value(QString("device"))),
        cycle(Catch::Generators::value(QString("root"))),
        cycle(Catch::Generators::value(QString("directory"))),
        cycle(Catch::Generators::value(QString("filename"))),
        cycle(Catch::Generators::value(copy_value(separator_component_value))),
        PathGeneratorConfiguration{}.set_directory_path_probability(1.0).set_has_trailing_separator_probability(0.0)
    );

    auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

    REQUIRE(!generated_path.endsWith(separator_component_value));
}

SCENARIO("Binding a path to a component range", "[Path][Bounds]") {
    GIVEN("A minimum amount of components") {
        auto minimum_components_amount = GENERATE(take(100, random(std::size_t{1}, std::size_t{100})));

        AND_GIVEN("A maximum amount of components that is greater or equal than the minimum amount of components") {
            auto maximum_components_amount = GENERATE_COPY(take(100, random(minimum_components_amount, std::size_t{100})));

            WHEN("A path is generated from those bounds") {
                QString countable_component_value{"a"};

                QString generated_path = GENERATE_COPY(
                    take(1,
                         path(
                            empty_string(),
                            empty_string(),
                            cycle(Catch::Generators::value(copy_value(countable_component_value))),
                            cycle(Catch::Generators::value(copy_value(countable_component_value))),
                            empty_string(),
                            PathGeneratorConfiguration{}.set_minimum_components_amount(minimum_components_amount).set_maximum_components_amount(maximum_components_amount)
                         )
                    )
                );

                THEN("The amount of non device, non root, non separator components in the generated path is in the range [minimum_components_amount, maximum_components_amount]") {
                    std::size_t components_amount{static_cast<std::size_t>(generated_path.count(countable_component_value))};

                    REQUIRE(components_amount >= minimum_components_amount);
                    REQUIRE(components_amount <= maximum_components_amount);
                }
            }
        }
    }
}

TEST_CASE(
    "When the maximum amount of components and the minimum amount of components are equal, all generated paths have the same amount of non device, non root, non separator components",
    "[Path][Bounds][SpecialCase]")
{
    auto components_amount = GENERATE(take(10, random(std::size_t{1}, std::size_t{100})));

    QString countable_component_value{"a"};
    QString generated_path = GENERATE_COPY(
        take(10,
            path(
                empty_string(),
                empty_string(),
                cycle(Catch::Generators::value(copy_value(countable_component_value))),
                cycle(Catch::Generators::value(copy_value(countable_component_value))),
                empty_string(),
                PathGeneratorConfiguration{}.set_minimum_components_amount(components_amount).set_maximum_components_amount(components_amount)
            )
        )
    );

    REQUIRE(static_cast<std::size_t>(generated_path.count(countable_component_value)) == components_amount);
}

SCENARIO("The format of a path", "[Path][Contents]") {
    GIVEN("A series of components generators") {
        // TODO: Could probably move this to the global scope to
        // lighen the tests.
        QString device_component_value{"device"};
        QString root_component_value{"root"};
        QString directory_component_value{"dir"};
        QString filename_component_value{"file"};
        QString separator_component_value{"sep"};

        auto device_component_generator = cycle(Catch::Generators::value(copy_value(device_component_value)));
        auto root_component_generator = cycle(Catch::Generators::value(copy_value(root_component_value)));
        auto directory_component_generator = cycle(Catch::Generators::value(copy_value(directory_component_value)));
        auto filename_component_generator = cycle(Catch::Generators::value(copy_value(filename_component_value)));
        auto separator_component_generator = cycle(Catch::Generators::value(copy_value(separator_component_value)));

        AND_GIVEN("A generator of paths using those components generator") {
            // TODO: We should actually randomize the configuration by
            // making a simple generator for it.
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("At most one device component is in the generated path") {
                    REQUIRE(generated_path.count(device_component_value) <= 1);
                }

                THEN("At most one root component is in the generated path") {
                    REQUIRE(generated_path.count(root_component_value) <= 1);
                }

                THEN("At most one filename component is in the generated path") {
                    REQUIRE(generated_path.count(filename_component_value) <= 1);
                }

                THEN("At least one non device, non root, non separator component is in the generated path") {
                    REQUIRE((generated_path.contains(directory_component_value) || generated_path.contains(filename_component_value)));
                }

                THEN("There is a separator component between any two successive directory components") {
                    // REMARK: To test this condition, which is not
                    // easy to test directly, as, if the generator is
                    // working as it should, the concept of successive
                    // directories stops existing.
                    // To test it, then, we split the condition into
                    // two parts, that are easier to test, that
                    // achieve the same effect.
                    // First, if all directories have a separator
                    // component between them, it is impossible to
                    // have a directory component that is directly
                    // followed by another directory component.
                    // Second, when this holds, any two directory
                    // components must have one or more non-directory
                    // components between them.
                    // For those directories that have exactly one
                    // component between them, it must be a separator.
                    // This is equivalent to the original condition as
                    // long as it is not allowed for anything else to
                    // be between two directory components that have
                    // exactly one component between them.
                    // This is true at the time of writing of this
                    // test, such that this will work correctly, but
                    // if this changes the test is invalidated.
                    // If a test for the original condition is found
                    // that is not contrived (as it is possible to
                    // test the original condition but it is a bit
                    // more complex than we would like the test to
                    // be), it should replace this current
                    // implementation to improve the resiliency of the
                    // test.
                    REQUIRE_FALSE(generated_path.contains(directory_component_value + directory_component_value));

                    auto successive_directories_re{
                    QRegularExpression(u"%1(%2)%3"_s.arg(directory_component_value)
                                                     .arg(QStringList{device_component_value, root_component_value, filename_component_value, separator_component_value}.join("|"))
                                                     .arg(directory_component_value)
                    )};

                    auto successive_directories_match(successive_directories_re.match(generated_path));
                    while (successive_directories_match.hasMatch()) {
                        auto in_between_component{successive_directories_match.captured(1)};

                        // TODO: Having this in a loop makes it so
                        // the amount of assertions will vary slightly
                        // per-run.
                        // It would be better to avoid this, even if
                        // it should not really be a problem
                        // generally.
                        // Try to find a better way to express this
                        // condition that does not require a loop.
                        // This could be as easy as just collection
                        // the results and then using a std::all_of.
                        REQUIRE(in_between_component == separator_component_value);

                        successive_directories_match = successive_directories_re.match(generated_path, successive_directories_match.capturedEnd(1));
                    }
                }


                THEN("There is a separator component between each successive directory and filename components") {
                    REQUIRE_FALSE(generated_path.contains(directory_component_value + filename_component_value));

                    auto successive_directory_filename_re{
                    QRegularExpression(u"%1(%2)%3"_s.arg(directory_component_value)
                                                     .arg(QStringList{device_component_value, root_component_value, filename_component_value, separator_component_value}.join("|"))
                                                     .arg(filename_component_value)
                    )};

                    auto successive_directory_filename_match(successive_directory_filename_re.match(generated_path));
                    while (successive_directory_filename_match.hasMatch()) {
                        auto in_between_component{successive_directory_filename_match.captured(1)};

                        REQUIRE(in_between_component == separator_component_value);

                        successive_directory_filename_match = successive_directory_filename_re.match(generated_path, successive_directory_filename_match.capturedEnd(1));
                    }
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that generates Multi-Device paths") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_multi_device_path_probability(1.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("Exactly one device component is in the generated path") {
                    REQUIRE(generated_path.count(device_component_value) == 1);

                    AND_THEN("The device component is the first component in the generated path") {
                        REQUIRE(generated_path.startsWith(device_component_value));
                    }
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that generates Absolute paths") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_absolute_path_probability(1.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("Exactly one root component is in the generated path") {
                    REQUIRE(generated_path.count(root_component_value) == 1);
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that generates Absolute paths that are not Multi-Device") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_multi_device_path_probability(0.0).set_absolute_path_probability(1.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("The root component is the first component in the generated path") {
                    REQUIRE(generated_path.startsWith(root_component_value));
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that generates Multi-Device, Absolute paths") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_multi_device_path_probability(1.0).set_absolute_path_probability(1.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("The root component succeeds the device component in the generated path") {
                    REQUIRE(generated_path.contains(device_component_value + root_component_value));
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that generates paths that are To a Directory and do not Have a Trailing Separator") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_directory_path_probability(1.0).set_has_trailing_separator_probability(0.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("The last component of in the path is a directory component") {
                    REQUIRE(generated_path.endsWith(directory_component_value));
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that generates paths that are To a Directory and Have a Trailing Separator") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_directory_path_probability(1.0).set_has_trailing_separator_probability(1.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("The last component in the path is a separator component that is preceded by a directory component") {
                    REQUIRE(generated_path.endsWith(directory_component_value + separator_component_value));
                }
            }
        }


        AND_GIVEN("A generator of paths using those components generator that generates paths that are To a File") {
            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_directory_path_probability(0.0)
            );

            WHEN("A path is generated from that generator") {
                auto generated_path = GENERATE_REF(take(10, std::move(path_generator)));

                THEN("Exactly one filename component is in the path") {
                    REQUIRE(generated_path.contains(filename_component_value));

                    AND_THEN("The filename component is the last component in the path") {
                        REQUIRE(generated_path.endsWith(filename_component_value));
                    }
                }
            }
        }
    }
}

// REMARK: [mayfail][distribution]
SCENARIO("Observing the distribution of paths based on their configuration", "[Path][Statistics][!mayfail]") {
    GIVEN("A series of components generators") {
        QString device_component_value{"device"};
        QString root_component_value{"root"};
        QString directory_component_value{"dir"};
        QString filename_component_value{"file"};
        QString separator_component_value{"sep"};

        auto device_component_generator = cycle(Catch::Generators::value(copy_value(device_component_value)));
        auto root_component_generator = cycle(Catch::Generators::value(copy_value(root_component_value)));
        auto directory_component_generator = cycle(Catch::Generators::value(copy_value(directory_component_value)));
        auto filename_component_generator = cycle(Catch::Generators::value(copy_value(filename_component_value)));
        auto separator_component_generator = cycle(Catch::Generators::value(copy_value(separator_component_value)));

        AND_GIVEN("A generator of paths using those components generator that produces paths that are Multi-Device with a probability of n") {
            double multi_device_path_probability = GENERATE(take(10, random(0.0, 1.0)));

            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_multi_device_path_probability(multi_device_path_probability)
            );

            WHEN("A certain amount of paths are generated from that generator") {
                auto paths = GENERATE_REF(take(1, chunk(10000, std::move(path_generator))));

                THEN("The amount of paths that are Multi-Device approximately respects the given probability and the amount of paths that are not approximately respects a probability of 1 - n") {
                    auto maybe_distribution_error{respects_distribution(
                        std::move(paths),
                        [&device_component_value](const QString& path){ return (path.startsWith(device_component_value)) ? "Multi-Device" : "Non Multi-Device"; },
                        [multi_device_path_probability](const QString& key){ return probability_to_percentage((key == "Multi-Device") ? multi_device_path_probability : 1 - multi_device_path_probability); }
                    )};

                    REQUIRE_FALSE(maybe_distribution_error);
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that produces paths that are Absolute with a probability of n") {
            double absolute_path_probability = GENERATE(take(10, random(0.0, 1.0)));

            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_absolute_path_probability(absolute_path_probability)
            );

            WHEN("A certain amount of paths are generated from that generator") {
                auto paths = GENERATE_REF(take(1, chunk(10000, std::move(path_generator))));

                THEN("The amount of paths that are Absolute approximately respects the given probability and the amount of paths that are Relative approximately respects a probability of 1 - n") {
                    auto maybe_distribution_error{respects_distribution(
                        std::move(paths),
                        [&root_component_value](const QString& path){ return (path.contains(root_component_value)) ? "Absolute" : "Relative"; },
                        [absolute_path_probability](const QString& key){ return probability_to_percentage((key == "Absolute") ? absolute_path_probability : 1 - absolute_path_probability); }
                    )};

                    REQUIRE_FALSE(maybe_distribution_error);
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that produces paths that are To a Directory with a probability of n") {
            double directory_path_probability = GENERATE(take(10, random(0.0, 1.0)));

            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_directory_path_probability(directory_path_probability)
            );

            WHEN("A certain amount of paths are generated from that generator") {
                auto paths = GENERATE_REF(take(1, chunk(10000, std::move(path_generator))));

                THEN("The amount of paths that are To a Directory approximately respects the given probability and the amount of paths that are To a File approximately respects a probability of 1 - n") {
                    auto maybe_distribution_error{respects_distribution(
                        std::move(paths),
                        [&filename_component_value](const QString& path){ return (path.contains(filename_component_value)) ? "To a File" : "To a Directory"; },
                        [directory_path_probability](const QString& key){ return probability_to_percentage((key == "To a Directory") ? directory_path_probability : 1 - directory_path_probability); }
                    )};

                    REQUIRE_FALSE(maybe_distribution_error);
                }
            }
        }

        AND_GIVEN("A generator of paths using those components generator that produces paths that are To a Directory with a probability of n to Have a Trailing Separator") {
            double has_trailing_separator_probability = GENERATE(take(10, random(0.0, 1.0)));

            auto path_generator = path(
                std::move(device_component_generator),
                std::move(root_component_generator),
                std::move(directory_component_generator),
                std::move(filename_component_generator),
                std::move(separator_component_generator),
                PathGeneratorConfiguration{}.set_directory_path_probability(1.0).set_has_trailing_separator_probability(has_trailing_separator_probability)
            );

            WHEN("A certain amount of paths are generated from that generator") {
                auto paths = GENERATE_REF(take(1, chunk(10000, std::move(path_generator))));

                THEN("The amount of paths that are Have a Trailing Separator approximately respects the given probability and the amount of paths that do not Have a Trailing Separator approximately respects a probability of 1 - n") {
                    auto maybe_distribution_error{respects_distribution(
                        std::move(paths),
                        [&separator_component_value](const QString& path){ return (path.endsWith(separator_component_value)) ? "Have a Trailing Separator" : "Doesn't Have a Trailing Separator"; },
                        [has_trailing_separator_probability](const QString& key){ return probability_to_percentage((key == "Have a Trailing Separator") ? has_trailing_separator_probability : 1 - has_trailing_separator_probability); }
                    )};

                    REQUIRE_FALSE(maybe_distribution_error);
                }
            }
        }
    }
}

TEST_CASE("The first component of the passed in device components generator is not lost", "[Path][GeneratorFirstElement][SpecialCase]") {
    QString device_component_generator_first_value{"device"};

    auto generated_path = GENERATE_COPY(take(1,
        path(
            values({device_component_generator_first_value, QString{""}}),
            empty_string(),
            empty_string(),
            empty_string(),
            empty_string(),
            PathGeneratorConfiguration{}
                .set_multi_device_path_probability(1.0)
                .set_minimum_components_amount(1)
                .set_maximum_components_amount(1)
        )
    ));

    REQUIRE(generated_path.contains(device_component_generator_first_value));
}

TEST_CASE("The first component of the passed in root components generator is not lost", "[Path][GeneratorFirstElement][SpecialCase]") {
    QString root_component_generator_first_value{"root"};

    auto generated_path = GENERATE_COPY(take(1,
        path(
            empty_string(),
            values({root_component_generator_first_value, QString{""}}),
            empty_string(),
            empty_string(),
            empty_string(),
            PathGeneratorConfiguration{}
                .set_absolute_path_probability(1.0)
                .set_minimum_components_amount(1)
                .set_maximum_components_amount(1)
        )
    ));

    REQUIRE(generated_path.contains(root_component_generator_first_value));
}

TEST_CASE("The first component of the passed in directory components generator is not lost", "[Path][GeneratorFirstElement][SpecialCase]") {
    QString directory_component_generator_first_value{"dir"};

    auto generated_path = GENERATE_COPY(take(1,
        path(
            empty_string(),
            empty_string(),
            values({directory_component_generator_first_value, QString{""}}),
            empty_string(),
            empty_string(),
            PathGeneratorConfiguration{}
                .set_directory_path_probability(1.0)
                .set_minimum_components_amount(1)
                .set_maximum_components_amount(1)
        )
    ));

    REQUIRE(generated_path.contains(directory_component_generator_first_value));
}

TEST_CASE("The first component of the passed in filename components generator is not lost", "[Path][GeneratorFirstElement][SpecialCase]") {
    QString filename_component_generator_first_value{"dir"};

    auto generated_path = GENERATE_COPY(take(1,
        path(
            empty_string(),
            empty_string(),
            empty_string(),
            values({filename_component_generator_first_value, QString{""}}),
            empty_string(),
            PathGeneratorConfiguration{}
                .set_directory_path_probability(0.0)
                .set_minimum_components_amount(1)
                .set_maximum_components_amount(1)
        )
    ));

    REQUIRE(generated_path.contains(filename_component_generator_first_value));
}

TEST_CASE("The first component of the passed in separator components generator is not lost", "[Path][GeneratorFirstElement][SpecialCase]") {
    QString separator_component_generator_first_value{"sep"};

    auto generated_path = GENERATE_COPY(take(1,
        path(
            empty_string(),
            empty_string(),
            empty_string(),
            empty_string(),
            values({separator_component_generator_first_value, QString{""}}),
            PathGeneratorConfiguration{}
                .set_directory_path_probability(0.0)
                .set_minimum_components_amount(2)
                .set_maximum_components_amount(2)
        )
    ));

    REQUIRE(generated_path.contains(separator_component_generator_first_value));
}

SCENARIO("Generating paths that are suitable to be used on POSIX systems", "[Path][POSIX][Content]") {
    GIVEN("A generator that generates Strings representing paths on a POSIX system that are portable") {
        auto path_generator = relaxed_portable_posix_path();

        WHEN("A path is generated from it") {
            auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

            THEN("The path is composed only by one or more characters in the class [-_./a-zA-Z0-9]") {
                REQUIRE(QRegularExpression{R"(\A[-_.\/a-zA-Z0-9]+\z)"}.match(generated_path).hasMatch());
            }
        }
    }
}

SCENARIO("Generating paths that are suitable to be used on Windows", "[Path][Windows][Content]") {
    GIVEN("A generator that generates Strings representing paths on a Windows system") {
        auto path_generator = traditional_dos_path();

        WHEN("A path is generated from it") {
            auto generated_path = GENERATE_REF(take(100, std::move(path_generator)));

            CAPTURE(generated_path);

            THEN("The path starts with an uppercase letter followed by a colon, a backward or forward slash or a character in the class [-_.a-zA-Z0-9]") {
                QRegularExpression beginning_re{"([A-Z]:|\\|\\/|[-_.a-zA-Z0-9])"};

                auto beginning_match{beginning_re.match(generated_path)};

                REQUIRE(beginning_match.hasMatch());

                generated_path.remove(0, beginning_match.capturedEnd());

                AND_THEN("The rest of the path is composed by zero or more characters in the class [-_./\\a-zA-Z0-9]") {
                    REQUIRE(QRegularExpression{R"(\A[-_.\/\\a-zA-Z0-9]*\z)"}.match(generated_path).hasMatch());
                }
            }
        }
    }
}
