# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_objcnamemangler LANGUAGES CXX OBJC)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT MACOS)
    return()
endif()

add_executable(objc_test_binary test.m)
target_link_libraries(objc_test_binary PRIVATE objc)
set_target_properties(objc_test_binary PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Create the Qt test
qt_internal_add_test(tst_objcnamemangler
    SOURCES
        tst_objcnamemangler.cpp
    LIBRARIES
        Qt::CorePrivate
)

add_dependencies(tst_objcnamemangler objc_test_binary)

