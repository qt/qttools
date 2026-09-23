# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qhelpsearchenginecore Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qhelpsearchenginecore LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qhelpsearchenginecore
    SOURCES
        tst_qhelpsearchenginecore.cpp
    DEFINES
        QT_USE_USING_NAMESPACE
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARIES
        Qt::Help
        Qt::Sql
)

qt_internal_extend_target(tst_qhelpsearchenginecore CONDITION QT_FEATURE_fullqthelp
    DEFINES
        TEST_FULLQTHELP
)
