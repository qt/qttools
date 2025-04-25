# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_i18n_source_language)

# Set up the project structure.
find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools)
qt_standard_project_setup()
set(native_ts_files "")

function(my_add_library target)
    add_library("${target}" STATIC lib.cpp)
    target_link_libraries("${target}" PRIVATE Qt6::Core)
endfunction()

# qt_add_lupdate with just the native translation.
my_add_library(lib1 STATIC lib.cpp)
qt_add_lupdate(
    SOURCE_TARGETS lib1
    PLURALS_TS_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib1_en.ts"
)

# qt_add_lupdate with just the native translation which has already been translated.
# check_ts_files.cmake checks that the numerus translations are preserved.
my_add_library(lib2 STATIC lib.cpp)
configure_file(lib2_en.ts.in lib2_en.ts COPYONLY)
qt_add_lupdate(
    SOURCE_TARGETS lib2
    PLURALS_TS_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib2_en.ts"
)

# qt_add_lupdate with regular translations and the native translation.
my_add_library(lib3 STATIC lib.cpp)
qt_add_lupdate(
    SOURCE_TARGETS lib3
    TS_FILES "${CMAKE_CURRENT_BINARY_DIR}/lib3_hi.ts" "${CMAKE_CURRENT_BINARY_DIR}/lib3_ho.ts"
    PLURALS_TS_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib3_en.ts"
)

# qt_add_translations with just the native translation.
my_add_library(lib4 STATIC lib.cpp)
qt_add_translations(
    TARGETS lib4
    SOURCE_TARGETS lib4
    PLURALS_TS_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib4_en.ts"
)

# qt_add_translations with regular translations and the native translation.
my_add_library(lib5 STATIC lib.cpp)
qt_add_translations(
    TARGETS lib5
    SOURCE_TARGETS lib5
    TS_FILES "${CMAKE_CURRENT_BINARY_DIR}/lib5_hi.ts" "${CMAKE_CURRENT_BINARY_DIR}/lib5_ho.ts"
    PLURALS_TS_FILE "${CMAKE_CURRENT_BINARY_DIR}/lib5_en.ts"
)

# Find out how many targets we created above.
get_directory_property(targets BUILDSYSTEM_TARGETS)
list(FILTER targets INCLUDE REGEX "^lib[0-9]+$")
list(LENGTH targets targets_length)

# Write the .ts file names to a file that's read by post_build_check.cmake.
set(ts_files "")
foreach(i RANGE 1 ${targets_length})
    list(APPEND ts_files "lib${i}_en.ts")
endforeach()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ts_files.txt" "${ts_files}")
