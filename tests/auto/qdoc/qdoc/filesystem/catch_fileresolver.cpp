// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_conversions/qdoc_catch_conversions.h>

#include <catch/catch.hpp>

#include <filesystem/fileresolver.h>

#include <catch_generators/generators/path_generator.h>

#include <vector>
#include <algorithm>
#include <random>

#include <QTemporaryDir>
#include <QFileInfo>
#include <QDir>
#include <QIODeviceBase>

SCENARIO("Inspecting the directories that will be used for searching", "[ResolvingFiles][Directory][Path][Canonicalization][Contents]") {
    GIVEN("Some collection of paths representing existing directories on the filesystem") {
        std::size_t directories_amount = GENERATE(take(10, random(2, 10)));

        std::vector<QTemporaryDir> working_directories(directories_amount);

        std::vector<DirectoryPath> directories;
        directories.reserve(directories_amount);

        std::transform(
            working_directories.begin(), working_directories.end(),
            std::back_inserter(directories),
            [](auto& dir){ return *DirectoryPath::refine(dir.path()); }
        );

        AND_GIVEN("That the collection of those paths is ordered and contains no duplicates") {
            std::sort(directories.begin(), directories.end());
            directories.erase(std::unique(directories.begin(), directories.end()), directories.end());

            WHEN("A mean of searching for files is obtained from that collection") {
                FileResolver file_resolver{std::vector(directories)};

                THEN("The collection of directories that will be used for searching is equivalent to the one from which a mean of searching for files was obtained") {
                    REQUIRE(file_resolver.get_search_directories() == directories);
                }
            }
        }

        AND_GIVEN("That the collection of those paths is potentially unordered but contains no duplicates") {
            std::sort(directories.begin(), directories.end());
            directories.erase(std::unique(directories.begin(), directories.end()), directories.end());

            std::shuffle(directories.begin(), directories.end(), std::mt19937{std::random_device{}()});

            WHEN("A mean of searching for files is obtained from that collection") {
                FileResolver file_resolver{std::vector(directories)};

                THEN("The collection of directories that will be used for searching is equivalent to the one from which a mean of searching for files was obtained if it was sorted") {
                    std::sort(directories.begin(), directories.end());

                    REQUIRE(file_resolver.get_search_directories() == directories);
                }
            }
        }

        AND_GIVEN("That the collection of those paths is ordered but contains duplicates") {
            directories.reserve(directories.size());

            std::transform(
                working_directories.begin(), working_directories.end(),
                std::back_inserter(directories),
                [](auto& dir){ return *DirectoryPath::refine(dir.path()); }
            );

            std::sort(directories.begin(), directories.end());

            WHEN("A mean of searching for files is obtained from that collection") {
                FileResolver file_resolver{std::vector(directories)};

                THEN("The collection of directories that will be used for searching is equivalent to the one from which a mean of searching for files was obtained if it contained no duplicates") {
                    directories.erase(std::unique(directories.begin(), directories.end()), directories.end());

                    REQUIRE(file_resolver.get_search_directories() == directories);
                }
            }
        }

        AND_GIVEN("That the collection of those paths is potentially unordered and contains duplicates") {
            directories.reserve(directories.size());

            std::transform(
                working_directories.begin(), working_directories.end(),
                std::back_inserter(directories),
                [](auto& dir){ return *DirectoryPath::refine(dir.path()); }
            );

            std::shuffle(directories.begin(), directories.end(), std::mt19937{std::random_device{}()});

            WHEN("A mean of searching for files is obtained from that collection") {
                FileResolver file_resolver{std::vector(directories)};

                THEN("The collection of directories that will be used for searching is equivalent to the one from which a mean of searching for files was obtained if it was sorted and it contained no duplicates") {
                    std::sort(directories.begin(), directories.end());
                    directories.erase(std::unique(directories.begin(), directories.end()), directories.end());

                    REQUIRE(file_resolver.get_search_directories() == directories);
                }
            }
        }
    }
}

SCENARIO("Finding a file based on some root search directories", "[ResolvingFiles][File][Path][Validation]") {
    // TODO: Rewrite those tests under a single setup. Be careful of
    // how this is done as Catch will rerun sections only under
    // specific condition such that we may incur in collisions for the
    // path if done incorrectly.
    GIVEN("Some directory on the filesystem") {
        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        DirectoryPath directory{*DirectoryPath::refine(working_directory.path())};

        AND_GIVEN("A mean of searching for files based on that directory") {
            FileResolver file_resolver{std::vector{directory}};

            AND_GIVEN("A relative path that does not represent an element on the filesystem that is reachable from that directory") {
                QString relative_path = GENERATE(filter([](auto& path){ return path != "." && path != ".."; }, take(100, qdoc::catch_generators::native_relative_path())));
                CAPTURE(relative_path);

                REQUIRE(!QFileInfo{working_directory.path() + '/' + relative_path}.exists());

                WHEN("The relative path is used as a query to resolve a file") {
                    auto maybe_resolved_file{file_resolver.resolve(relative_path)};

                    THEN("The query cannot be resolved") {
                        REQUIRE(!maybe_resolved_file);
                    }
                }
            }
        }
    }

    GIVEN("Some directory on the filesystem") {
        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        DirectoryPath directory{*DirectoryPath::refine(working_directory.path())};

        AND_GIVEN("A mean of searching for files based on that directory") {
            FileResolver file_resolver{std::vector{directory}};

            AND_GIVEN("A relative path that represents an existing directory on the filesystem that is reachable from that directory") {
                QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_directory_path()));
                CAPTURE(relative_path);

                REQUIRE(QDir{working_directory.path()}.mkpath(relative_path));

                WHEN("The relative path is used as a query to resolve a file") {
                    auto maybe_resolved_file{file_resolver.resolve(relative_path)};

                    THEN("The query cannot be resolved") {
                        REQUIRE(!maybe_resolved_file);
                    }
                }
            }
        }
    }


    GIVEN("Some directory on the filesystem") {
        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        DirectoryPath directory{*DirectoryPath::refine(working_directory.path())};

        AND_GIVEN("A mean of searching for files based on that directory") {
            FileResolver file_resolver{std::vector{directory}};

            AND_GIVEN("A relative path that represents an existing file on the filesystem that is reachable from that directory") {
                QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_file_path()));
                CAPTURE(relative_path);

                REQUIRE(QDir{working_directory.path()}.mkpath(QFileInfo{relative_path}.path()));
                REQUIRE(QFile{working_directory.path() + "/" + relative_path}.open(QIODeviceBase::ReadWrite | QIODeviceBase::NewOnly));

                WHEN("The relative path is used as a query to resolve a file") {
                    auto maybe_resolved_file{file_resolver.resolve(relative_path)};

                    THEN("The query can be resolved") {
                        REQUIRE(maybe_resolved_file);
                    }
                }
            }
        }
    }

    GIVEN("Some directories on the filesystem") {
        std::size_t directories_amount = GENERATE(take(10, random(2, 10)));

        std::vector<QTemporaryDir> working_directories(directories_amount);
        REQUIRE(std::all_of(working_directories.cbegin(), working_directories.cend(), [](auto& dir){ return dir.isValid(); }));

        std::vector<DirectoryPath> directories;
        directories.reserve(directories_amount);

        std::transform(
            working_directories.begin(), working_directories.end(),
            std::back_inserter(directories),
            [](auto& dir){ return *DirectoryPath::refine(dir.path()); }
        );

        AND_GIVEN("A relative path that represents an existing file on the filesystem that is reachable from exactly one of those directories") {
            QString relative_path = GENERATE(take(10, qdoc::catch_generators::native_relative_file_path()));
            CAPTURE(relative_path);

            std::size_t containing_directory_index = GENERATE_COPY(take(1, random(std::size_t{0}, directories_amount - 1)));
            CAPTURE(containing_directory_index);
            CAPTURE(working_directories[containing_directory_index].path());

            REQUIRE(QDir{working_directories[containing_directory_index].path()}.mkpath(QFileInfo{relative_path}.path()));
            REQUIRE(QFile{working_directories[containing_directory_index].path() + "/" + relative_path}.open(QIODeviceBase::ReadWrite | QIODeviceBase::NewOnly));

            AND_GIVEN("A mean of searching for files based on all of those directories") {
                FileResolver file_resolver{std::move(directories)};

                WHEN("The relative path is used as a query to resolve a file") {
                    auto maybe_resolved_file{file_resolver.resolve(relative_path)};

                    THEN("The query can be resolved") {
                        REQUIRE(maybe_resolved_file);
                    }
                }
            }
        }
    }
}

SCENARIO("Inspecting the content of a file that was resolved", "[ResolvingFiles][File][Path][Validation][Contents]") {
    GIVEN("A mean of resolving files based on some directory") {
        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        DirectoryPath directory{*DirectoryPath::refine(working_directory.path())};

        FileResolver file_resolver{std::vector{directory}};

        AND_GIVEN("A relative path that represents an existing file on the filesystem that is reachable from that directory") {
            QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_file_path()));
            CAPTURE(relative_path);

            REQUIRE(QDir{working_directory.path()}.mkpath(QFileInfo{relative_path}.path()));
            REQUIRE(QFile{working_directory.path() + "/" + relative_path}.open(QIODeviceBase::ReadWrite | QIODeviceBase::NewOnly));

            WHEN("A file is resolved using that path as a query") {
                auto resolved_file{*file_resolver.resolve(relative_path)};

                THEN("The resolved file contains the query that was used to resolve it") {
                    REQUIRE(resolved_file.get_query() == relative_path);
                }

                THEN("The resolved file contains a canonicalized path pointing to the resolved file") {
                    REQUIRE(resolved_file.get_path() == QFileInfo{working_directory.path() + "/" + relative_path}.canonicalFilePath());
                }
            }
        }
    }
}

TEST_CASE(
    "When a query can be resolved in more than one search directory, it is resolved in the greatest lower bound of the set of directories",
    "[ResolvingFiles][File][Path][Validation][SpecialCase]"
) {
    std::size_t directories_amount = GENERATE(take(10, random(2, 10)));

    QString relative_path = GENERATE(take(10, qdoc::catch_generators::native_relative_file_path()));
    CAPTURE(relative_path);

    std::vector<QTemporaryDir> working_directories(directories_amount);
    REQUIRE(std::all_of(working_directories.cbegin(), working_directories.cend(), [](auto& dir){ return dir.isValid(); }));

    for (const auto& directory : working_directories) {
        CAPTURE(directory.path());
        REQUIRE(QDir{directory.path()}.mkpath(QFileInfo{relative_path}.path()));
        REQUIRE(QFile{directory.path() + "/" + relative_path}.open(QIODeviceBase::ReadWrite | QIODeviceBase::NewOnly));
    }

    std::vector<DirectoryPath> directories;
    directories.reserve(directories_amount);

    std::transform(
        working_directories.begin(), working_directories.end(),
        std::back_inserter(directories),
        [](auto& dir){ return *DirectoryPath::refine(dir.path()); }
    );

    FileResolver file_resolver{std::move(directories)};
    auto resolved_file{*file_resolver.resolve(relative_path)};

    auto& greatest_lower_bound{*std::min_element(file_resolver.get_search_directories().cbegin(), file_resolver.get_search_directories().cend())};

    REQUIRE(
        resolved_file.get_path()
        ==
        QFileInfo{greatest_lower_bound.value() + "/" + relative_path}.canonicalFilePath()
    );
}
