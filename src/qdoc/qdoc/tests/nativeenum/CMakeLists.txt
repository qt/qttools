# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_inodeinterface Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_nativeenum LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_nativeenum
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tst_nativeenum.cpp

    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}/../../src/
)
