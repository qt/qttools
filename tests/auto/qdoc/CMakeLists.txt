# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND QT_FEATURE_debug_and_release)
    message(WARNING "qdoc tests will not be built in this configuration.")
    return()
endif()

set(QDOC_PROJECT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../src/qdoc/)
set(QDOC_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../src/qdoc/qdoc/)
set(QDOC_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../src/qdoc/qdoc/)

if(QT_BUILD_STANDALONE_TESTS)
    include(${QDOC_PROJECT_DIRECTORY}/catch_generators/tests/CMakeLists.txt)
endif()

add_subdirectory(qdoc)
add_subdirectory(config)
add_subdirectory(generatedoutput)
add_subdirectory(qdoccommandlineparser)
add_subdirectory(utilities)
