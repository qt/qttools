# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_mangle_objc_symbols_codesign LANGUAGES CXX OBJC)

find_package(Qt6 REQUIRED COMPONENTS Core BuildTools)

# Build a simple Objective-C test binary
add_executable(test_objc_binary test.m)
target_link_libraries(test_objc_binary PRIVATE objc)

# Test qt6_mangle_objc_symbols with CODESIGN option
qt6_mangle_objc_symbols(test_objc_binary
    OLD_NAMESPACE "_Suffix"
    NEW_NAMESPACE "_SUFFIX"
    CODESIGN
)
