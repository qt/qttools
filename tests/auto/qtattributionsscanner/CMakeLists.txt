# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qtattributionsscanner Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qtattributionsscanner LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qtattributionsscanner
    SOURCES
        tst_qtattributionsscanner.cpp
)

target_compile_definitions(tst_qtattributionsscanner
    PRIVATE QTTOOLS_LICENSES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../../LICENSES"
)
