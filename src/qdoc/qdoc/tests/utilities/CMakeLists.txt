# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_utilities Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_utilities LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_utilities
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tst_utilities.cpp

        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/utilities.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/config.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/location.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/qdoccommandlineparser.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}/../../src/
)
