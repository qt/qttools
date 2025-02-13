# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_i18n_auto_ts_file_names)

# Set up the project structure.
find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools)
qt_standard_project_setup()

function(my_add_library target)
    qt6_add_library("${target}" ${ARGN})
    target_link_libraries("${target}" PRIVATE Qt6::Core)
endfunction()

# Remove .ts files from older test runs.
file(GLOB_RECURSE old_ts_files "${CMAKE_CURRENT_SOURCE_DIR}/*.ts")
foreach(f IN LISTS old_ts_files)
    message("Removing: ${f}")
    file(REMOVE "${f}")
endforeach()

set(expected_files "")
set(unexpected_files "")

# Check defaults for the deferred call.
my_add_library(lib1 STATIC lib.cpp)
set(QT_I18N_TRANSLATED_LANGUAGES te st)
qt_add_translations(TARGETS lib1
    TS_FILES_OUTPUT_VARIABLE lib1_ts_files
)
set(lib1_expected_ts_files
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_en.ts"            # plurals-only file
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_te.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_st.ts")
if(lib1_ts_files STREQUAL lib1_expected_ts_files)
    list(APPEND expected_files ${lib1_ts_files})
else()
    message(FATAL_ERROR "expected .ts files:\n ${expected_files}"
        "\nactual .ts files:\n" ${lib1_ts_files}
    )
endif()

# Check defaults for the immediate call.
my_add_library(lib2 STATIC lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE hu)
set(QT_I18N_TRANSLATED_LANGUAGES hi ho)
qt_add_translations(TARGETS lib2 IMMEDIATE_CALL)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_hu.ts"            # plurals-only file
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_hi.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_ho.ts")

# Check defaults for deferred/immediate call in a subdir.
add_subdirectory(subdir)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/subdir/test_i18n_auto_ts_file_names_eo.ts"    # plurals-only file
    "${CMAKE_CURRENT_SOURCE_DIR}/subdir/test_i18n_auto_ts_file_names_de.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/subdir/test_i18n_auto_ts_file_names_ee.ts"    # plurals-only file
    "${CMAKE_CURRENT_SOURCE_DIR}/subdir/test_i18n_auto_ts_file_names_da.ts"
)

# Check whether TS_FILE_BASE works.
my_add_library(lib5 STATIC lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE ne)
set(QT_I18N_TRANSLATED_LANGUAGES no)
qt_add_translations(TARGETS lib5 TS_FILE_BASE lib5)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/lib5_ne.ts"                                    # plurals-only file
    "${CMAKE_CURRENT_SOURCE_DIR}/lib5_no.ts")

# Check whether TS_FILE_DIR works.
my_add_library(lib6 STATIC lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE so)                                                 # plurals-only file
set(QT_I18N_TRANSLATED_LANGUAGES sv)
qt_add_translations(TARGETS lib6 TS_FILE_DIR translations)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/test_i18n_auto_ts_file_names_sv.ts")

# Check whether TS_FILE_BASE and TS_FILE_DIR work together.
my_add_library(lib7 STATIC lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE fy)
set(QT_I18N_TRANSLATED_LANGUAGES fi)
qt_add_translations(TARGETS lib7 TS_FILE_BASE lib7 TS_FILE_DIR translations)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/lib7_fy.ts"                       # plurals-only file
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/lib7_fi.ts")

# Check defaults for the deferred call with just the native language set.
my_add_library(lib8 STATIC lib.cpp)
set(QT_I18N_TRANSLATED_LANGUAGES "")
set(QT_I18N_SOURCE_LANGUAGE nl)
qt_add_translations(TARGETS lib8)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_nl.ts")

# Check whether we can turn off the generation of the plurals-only file.
my_add_library(lib10 STATIC lib.cpp)
set(QT_I18N_TRANSLATED_LANGUAGES cy)
set(QT_I18N_SOURCE_LANGUAGE an)
qt_add_translations(TARGETS lib10 NO_GENERATE_PLURALS_TS_FILE)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_cy.ts")
list(APPEND unexpected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/test_i18n_auto_ts_file_names_an.ts")

# Check whether TS_OUTPUT_DIRECTORY works.
my_add_library(lib11 STATIC lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE aa)
set(QT_I18N_TRANSLATED_LANGUAGES ab)
qt_add_translations(TARGETS lib11 TS_OUTPUT_DIRECTORY translations)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/test_i18n_auto_ts_file_names_aa.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/test_i18n_auto_ts_file_names_ab.ts")

# Check whether TS_FILE_BASE and TS_OUTPUT_DIRECTORY work together.
my_add_library(lib12 STATIC lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE af)
set(QT_I18N_TRANSLATED_LANGUAGES ak)
qt_add_translations(TARGETS lib12 TS_FILE_BASE lib12 TS_OUTPUT_DIRECTORY translations)
list(APPEND expected_files
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/lib12_af.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/lib12_ak.ts")

function(check_ts_file_paths)
    foreach(filepath IN LISTS expected_files)
        if(NOT EXISTS "${filepath}")
            message(FATAL_ERROR "Expected file '${filepath}' does not exist.")
        endif()
    endforeach()
    foreach(filepath IN LISTS unexpected_files)
        if(EXISTS "${filepath}")
            message(FATAL_ERROR "File '${filepath}' unexpectedly exists.")
        endif()
    endforeach()
endfunction()

if(CMAKE_VERSION VERSION_LESS "3.19")
    check_ts_file_paths()
else()
    cmake_language(DEFER CALL check_ts_file_paths)
endif()
