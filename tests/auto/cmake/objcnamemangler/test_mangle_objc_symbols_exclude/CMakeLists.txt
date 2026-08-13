# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_mangle_objc_symbols_exclude LANGUAGES CXX OBJC)

find_package(Qt6 REQUIRED COMPONENTS Core ToolsTools)

# Build a simple Objective-C test binary with multiple classes
add_executable(test_objc_binary test.m)
target_link_libraries(test_objc_binary PRIVATE objc)

# Test qt6_mangle_objc_symbols with EXCLUDE_CLASSES option
# This should mangle TestClass_Manager but NOT TestClass_Helper
qt6_mangle_objc_symbols(test_objc_binary
    OLD_NAMESPACE "TestClass"
    NEW_NAMESPACE "TestKlass"
    EXCLUDE_CLASSES TestClass_Helper
    CODESIGN
)
