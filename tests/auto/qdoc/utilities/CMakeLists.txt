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
        ${QDOC_SOURCE_DIRECTORY}/utilities.cpp
        tst_utilities.cpp
    INCLUDE_DIRECTORIES
        ${QDOC_INCLUDE_DIRECTORY}
)
