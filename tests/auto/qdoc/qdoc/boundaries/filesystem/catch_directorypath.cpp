// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <catch_conversions/qdoc_catch_conversions.h>

#include <catch/catch.hpp>

#include <boundaries/filesystem/directorypath.h>

#include <catch_generators/generators/path_generator.h>

#include <QFileInfo>
#include <QTemporaryDir>
#include <QDir>
#include <QIODeviceBase>
#include <QRegularExpression>

SCENARIO("Obtaining a DirectoryPath", "[DirectoryPath][Boundaries][Validation][Canonicalization][Path]") {

    GIVEN("Any string representing a path that does not represent an existing element on the filesystem") {
        QString path = GENERATE(take(100, filter([](auto path){ return !QFileInfo{path}.exists(); }, qdoc::catch_generators::native_path())));
        CAPTURE(path);

        WHEN("A DirectoryPath instance is requested from that string") {
            auto maybe_directory_path{DirectoryPath::refine(path)};

            THEN("A DirectoryPath instance is not obtained") {
                REQUIRE(!maybe_directory_path);
            }
        }
    }

    GIVEN("Any string representing a path to a file") {
        QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_file_path()));
        CAPTURE(relative_path);

        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        QString path_to_file = working_directory.path() + "/" + relative_path;

        AND_GIVEN("That the path represents an existing file on the filesystem") {
            REQUIRE(QDir{working_directory.path()}.mkpath(QFileInfo{relative_path}.path()));
            REQUIRE(QFile{path_to_file}.open(QIODeviceBase::ReadWrite | QIODevice::NewOnly));

            WHEN("A DirectoryPath instance is requested from that string") {
                auto maybe_directory_path{DirectoryPath::refine(path_to_file)};

                THEN("A DirectoryPath instance is not obtained") {
                    REQUIRE(!maybe_directory_path);
                }
            }
        }
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)

    GIVEN("Any string representing a path to a directory") {
        // REMARK: [relative-component-permissions]
        // For tests where we change the permissions of the path, we
        // want to avoid relative components in a final position.
        // Relative components are actual objects on the filesystem in
        // *nix systems.
        // What this means is that to perform some operations on them,
        // such as changing permissions, we need the correct
        // permission in their containing or parent directory.
        // When we change permissions for those files, the permissions
        // for their containing or parent directory is actually
        // changed.
        // Depending on the way in which the permissions where
        // changed, it may then be impossible to change them back, as
        // the containing or parent directory might not provide the
        // necessary permission to read or change the nodes that it
        // contains.
        // For tests in particular, this means that we are not able to
        // ensure that the correct permissions will be available for
        // the cleanup of the temporary directories that we need for
        // testing.
        // To avoid this situation, we filter out those paths that end
        // in a relative component.
        QString relative_path = GENERATE(take(100,
            filter(
                [](QString path){
                    QString last_component = path.split(QRegularExpression{R"(\/+)"}, Qt::SkipEmptyParts).last();
                    return (last_component != ".") && (last_component != "..");
                },
                qdoc::catch_generators::native_relative_file_path()
            )
        ));
        CAPTURE(relative_path);

        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        QString path_to_directory = working_directory.path() + "/" + relative_path;

        AND_GIVEN("That the path represents an existing directory on the filesystem") {
            REQUIRE(QDir{working_directory.path()}.mkpath(relative_path));

            AND_GIVEN("That the directory represented by the path is not readable") {
                REQUIRE(QFile::setPermissions(path_to_directory, QFileDevice::WriteOwner  |
                                                                  QFileDevice::ExeOwner    |
                                                                  QFileDevice::WriteGroup  |
                                                                  QFileDevice::ExeGroup    |
                                                                  QFileDevice::WriteOther  |
                                                                  QFileDevice::ExeOther));

                CAPTURE(QFileInfo{path_to_directory}.isReadable());

                WHEN("A DirectoryPath instance is requested from that string") {
                    auto maybe_directory_path{DirectoryPath::refine(path_to_directory)};

                    THEN("A DirectoryPath instance is not obtained") {
                        // REMARK: [temporary_directory_cleanup]
                        CHECK(!maybe_directory_path);
                        REQUIRE(QFile::setPermissions(path_to_directory, QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser));
                    }
                }
            }

            AND_GIVEN("That the directory represented by the path is not executable") {
                REQUIRE(QFile::setPermissions(path_to_directory, QFileDevice::WriteOwner  |
                                                                  QFileDevice::ReadOwner  |
                                                                  QFileDevice::WriteGroup |
                                                                  QFileDevice::ReadGroup  |
                                                                  QFileDevice::WriteOther |
                                                                  QFileDevice::ReadOther));

                WHEN("A DirectoryPath instance is requested from that string") {
                    auto maybe_directory_path{DirectoryPath::refine(path_to_directory)};

                    THEN("A DirectoryPath instance is not obtained") {
                        // REMARK: [temporary_directory_cleanup]
                        CHECK(!maybe_directory_path);
                        REQUIRE(QFile::setPermissions(path_to_directory, QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser));
                    }
                }
            }

            AND_GIVEN("That the directory represented by the path is readable and executable") {
                REQUIRE(QFile::setPermissions(path_to_directory, QFileDevice::ReadOwner  |
                                                                  QFileDevice::ExeOwner  |
                                                                  QFileDevice::ReadGroup |
                                                                  QFileDevice::ExeGroup  |
                                                                  QFileDevice::ReadOther |
                                                                  QFileDevice::ExeOther));

                WHEN("A DirectoryPath instance is requested from that string") {
                    auto maybe_directory_path{DirectoryPath::refine(path_to_directory)};

                    THEN("A DirectoryPath instance is obtained") {
                        // REMARK: [temporary_directory_cleanup]
                        // We restore all permission to
                        // ensure that the temporary directory can be
                        // automatically cleaned up.
                        CHECK(maybe_directory_path);
                        REQUIRE(QFile::setPermissions(path_to_directory, QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser));
                    }
                }
            }
        }
    }

#endif

}

SCENARIO("Inspecting the contents of a DirectoryPath", "[DirectoryPath][Boundaries][Canonicalization][Path][Contents]") {
    GIVEN("Any string representing a path from which a DirectoryPath instance can be obtained") {
        QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_directory_path()));
        CAPTURE(relative_path);

        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        QString path_to_directory = QFileInfo{working_directory.path() + "/" + relative_path}.filePath();

        REQUIRE(QDir{working_directory.path()}.mkpath(relative_path));

        AND_GIVEN("A DirectoryPath instance obtained from that path") {
            auto maybe_directory_path{DirectoryPath::refine(path_to_directory)};
            REQUIRE(maybe_directory_path);

            auto directory_path{*maybe_directory_path};

            WHEN("The path that the DirectoryPath contains is inspected") {
                auto path{directory_path.value()};

                THEN("That path is the same as the canonicazlied version of the path that the DirectoryPath was built from") {
                    REQUIRE(path == QFileInfo(path_to_directory).canonicalFilePath());
                }
            }
        }
    }
}
