# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND QT_FEATURE_debug_and_release)
    message(WARNING "qdoc tests will not be built in this configuration.")
    return()
endif()

set(QDOC_PROJECT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../src/qdoc/)
set(QDOC_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../src/qdoc/qdoc/src/qdoc)
set(QDOC_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../src/qdoc/qdoc/src/)

# Mirrors the definition for test targets from the source directory to the
# "tests" directory.
# The CI for the Qt project is set up so that, when tests are run in CI, only
# the "tests" directory under the root of a repo is configured and test targets
# are later run over an already built Qt (with everything exported from the
# various src directories).
# This means that any target that is defined outside the "tests" directory in
# the root of the repo will not be picked up for testing by CI.
#
# As QDoc is currently organizing so that its part of the "src" directory
# contains everything related to its of its subprojects, tests included, our
# tests targets will not be checked by CI.
#
# To avoid this issue, each test target is defined in its own self-contained
# "CMakeLists.txt" file under its correct place in the "src" directory and is
# later included here to ensure that it is picked up when Ci run the tests for
# the qttools repository.
#
# Outside of CI, both the "src" and "tests" directory will be configured and we
# need to avoid repeating the targets definition.
#
# When CI is running its tests, it will set `QT_BUILD_STANDALONE_TESTS`, which
# we can depend on to filter the inclusions.
if(QT_BUILD_STANDALONE_TESTS)
    include(${QDOC_PROJECT_DIRECTORY}/catch_generators/tests/CMakeLists.txt)

    # Add all tests in src/qdoc/qdoc/tests:
    file(GLOB qdoc_tests_dir_content LIST_DIRECTORIES true "${QDOC_PROJECT_DIRECTORY}/qdoc/tests/*")
    foreach(path IN LISTS qdoc_tests_dir_content)
        if(EXISTS "${path}/CMakeLists.txt")
            get_filename_component(test_directory "${path}" NAME)
            add_subdirectory("${path}" "${test_directory}")
        endif()
    endforeach()
endif()
