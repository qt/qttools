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

#include <qdoc_catch_conversions.hpp>

#include <catch.hpp>

#include <boundaries/filesystem/filepath.hpp>

#include <qdoc_catch_generators.hpp>

#include <QFileInfo>
#include <QTemporaryDir>
#include <QDir>
#include <QIODeviceBase>

SCENARIO("Obtaining a FilePath", "[FilePath][Boundaries][Validation][Canonicalization][Path]") {

    GIVEN("Any string representing a path that does not represent an existing element on the filesystem") {
        QString path = GENERATE(take(100, filter([](auto path){ return !QFileInfo{path}.exists(); }, qdoc::catch_generators::native_path())));

        WHEN("A FilePath instance is requested from that string") {
            auto maybe_filepath{FilePath::refine(path)};

            THEN("A FilePath instance is not obtained") {
                REQUIRE(!maybe_filepath);
            }
        }
    }

    GIVEN("Any string representing a path to a directory") {
        QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_directory_path()));

        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        QString path_to_directory = working_directory.path() + "/" + relative_path;

        AND_GIVEN("That the path represents an existing directory on the filesystem") {
            REQUIRE(QDir{working_directory.path()}.mkpath(relative_path));

            WHEN("A FilePath instance is requested from that string") {
                auto maybe_filepath{FilePath::refine(path_to_directory)};

                THEN("A FilePath instance is not obtained") {
                    REQUIRE(!maybe_filepath);
                }
            }
        }
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)

    GIVEN("Any string representing a path to a file") {
        QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_file_path()));

        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        QString path_to_file = working_directory.path() + "/" + relative_path;

        AND_GIVEN("That the path represents an existing file on the filesystem") {
            REQUIRE(QDir{working_directory.path()}.mkpath(QFileInfo{relative_path}.path()));
            REQUIRE(QFile{path_to_file}.open(QIODeviceBase::ReadWrite | QIODeviceBase::NewOnly));

            AND_GIVEN("That the file represented by the path is not readable") {
                REQUIRE(QFile::setPermissions(path_to_file, QFileDevice::WriteOwner  |
                                                           QFileDevice::ExeOwner    |
                                                           QFileDevice::WriteGroup  |
                                                           QFileDevice::ExeGroup    |
                                                           QFileDevice::WriteOther  |
                                                           QFileDevice::ExeOther));

                WHEN("A FilePath instance is requested from that string") {
                    auto maybe_filepath{FilePath::refine(path_to_file)};

                    THEN("A FilePath instance is not obtained") {
                        // REMARK: [temporary_directory_cleanup]
                        CHECK(!maybe_filepath);
                        REQUIRE(QFile::setPermissions(path_to_file, QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser));
                    }
                }
            }

            AND_GIVEN("That the file represented by the path is readable") {
                REQUIRE(QFile::setPermissions(path_to_file, QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther));

                WHEN("A FilePath instance is requested from that string") {
                    auto maybe_filepath{FilePath::refine(path_to_file)};

                    THEN("A FilePath instance is obtained") {
                        // REMARK: [temporary_directory_cleanup]
                        CHECK(maybe_filepath);
                        REQUIRE(QFile::setPermissions(path_to_file, QFileDevice::WriteUser | QFileDevice::ReadUser | QFileDevice::ExeUser));
                    }
                }
            }
        }
    }

#endif

}

SCENARIO("Inspecting the contents of a FilePath", "[FilePath][Boundaries][Canonicalization][Path][Contents]") {
    GIVEN("Any string representing a path from which a FilePath instance can be obtained") {
        QString relative_path = GENERATE(take(100, qdoc::catch_generators::native_relative_file_path()));

        QTemporaryDir working_directory{};
        REQUIRE(working_directory.isValid());

        QString path_to_file = QFileInfo{working_directory.path() + "/" + relative_path}.filePath();

        REQUIRE(QDir{working_directory.path()}.mkpath(QFileInfo{relative_path}.path()));
        REQUIRE(QFile{path_to_file}.open(QIODeviceBase::ReadWrite | QIODeviceBase::NewOnly));

        AND_GIVEN("A FilePath instance obtained from that path") {
            auto maybe_filepath{FilePath::refine(path_to_file)};
            REQUIRE(maybe_filepath);

            auto filepath{*maybe_filepath};

            WHEN("The path that the FilePath contains is inspected") {
                auto path{filepath.value()};

                THEN("That path is the same as the canonicazlied version of the path that the FilePath was built from") {
                    REQUIRE(path == QFileInfo(path_to_file).canonicalFilePath());
                }
            }
        }
    }
}
