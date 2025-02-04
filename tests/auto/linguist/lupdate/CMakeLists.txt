# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_lupdate LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

#####################################################################
## tst_lupdate Test:
#####################################################################

qt_internal_add_test(tst_lupdate
    SOURCES
        tst_lupdate.cpp
    LIBRARIES
        Qt::Tools
        Qt::CorePrivate
)
