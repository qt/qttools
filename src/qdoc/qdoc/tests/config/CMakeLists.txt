# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_config Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_config LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_config
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tst_config.cpp

        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/config.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/location.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/qdoccommandlineparser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/utilities.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}/../../src/
)
