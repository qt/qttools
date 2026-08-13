# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_mangle_objc_symbols_qt_namespace LANGUAGES CXX OBJCXX)

find_package(Qt6 REQUIRED COMPONENTS Core BuildTools)

# Build a simple Objective-C++ test binary that uses Qt
add_executable(test_objc_binary test.m)
target_link_libraries(test_objc_binary PRIVATE Qt6::Core)

# Test qt6_mangle_objc_symbols with USE_QT_NAMESPACE_AS_OLD mode
# The Qt namespace will be read from Qt::Core's QT_NAMESPACE property
# and the replacement will be padded with underscores if needed
qt6_mangle_objc_symbols(test_objc_binary
    USE_QT_NAMESPACE_AS_OLD
    NEW_NAMESPACE "MyNS"
    CODESIGN
)
