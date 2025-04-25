# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_update_translation_macro)

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

function(create_app target)
    add_executable(${target}
        myi18nobject.cpp)

    target_include_directories(${target}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/some_dir)

    target_link_libraries(${target} Qt6::Core)
endfunction()

function(expect_target target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Expected target '${target}' does not exist.")
    endif()
endfunction()

function(not_expect_target target)
    if(TARGET ${target})
        message(FATAL_ERROR "Target '${target}' exists unexpectedly.")
    endif()
endfunction()

function(expect_files_in_list list_var)
    set(file_names "")
    foreach(path IN LISTS "${list_var}")
        get_filename_component(file_name "${path}" NAME)
        list(APPEND file_names "${file_name}")
    endforeach()
    set(found_file_names "")
    foreach(expected_file_name IN LISTS ARGN)
        list(FIND file_names "${expected_file_name}" idx)
        if(idx EQUAL -1)
            message(FATAL_ERROR "Expected file name '${expected_file_name}' is not in "
                "'${list_var}'. Its value is '${${list_var}}'.")
        endif()
        list(APPEND found_file_names "${expected_file_name}")
    endforeach()
    list(REMOVE_ITEM file_names ${found_file_names})
    list(LENGTH file_names n)
    if(NOT n EQUAL 0)
        message(FATAL_ERROR "Unexpected file names in '${list_var}': ${file_names}\n"
            "Value of '${list_var}' is '${${list_var}}'.")
    endif()
endfunction()

function(expect_string_value variable value)
    if(NOT "${${variable}}" STREQUAL "${value}")
        message(FATAL_ERROR "Unexpected string value in variable '${variable}': ${value}\n"
            "Value of '${variable}' is '${${variable}}'."
        )
    endif()
endfunction()

# Test NO_GLOBAL_TARGET for lupdate.
# Also, use the old signature where the first argument is the target.
create_app(app1)
qt6_add_lupdate(app1
    NO_GLOBAL_TARGET
    TS_FILES myobject_de.ts)
expect_target(test_update_translation_macro_lupdate)
not_expect_target(app1_lrelease)
not_expect_target(update_translations)
not_expect_target(release_translations)

# Test NO_GLOBAL_TARGET for lrelease.
create_app(app2)
qt6_add_lrelease(app2
    NO_GLOBAL_TARGET
    TS_FILES myobject_de.ts myobject_en.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
not_expect_target(app2_lupdate)
expect_target(test_update_translation_macro_lrelease)
not_expect_target(update_translations)
not_expect_target(release_translations)
expect_files_in_list(qm_files myobject_de.qm myobject_en.qm)

# Typical usage of qt_add_lupdate/qt_add_lrelease. Pass some options for good measure.
create_app(app3)
qt6_add_lupdate(
    SOURCE_TARGETS app1 app2 app3
    TS_FILES myobject_no.ts myobject_fi.ts
    OPTIONS -source-language en_US)
qt6_add_lrelease(app3
    TS_FILES myobject_no.ts myobject_fi.ts
    LRELEASE_TARGET app3_lrelease
    OPTIONS -compress
    QM_FILES_OUTPUT_VARIABLE qm_files2)
expect_target(test_update_translation_macro_lupdate1)
expect_target(app3_lrelease)
expect_target(release_translations)
expect_files_in_list(qm_files2 myobject_no.qm myobject_fi.qm)

# Now do the same with qt6_add_translations.
create_app(app4)
qt6_add_translations(
    IMMEDIATE_CALL
    TARGETS app4
    SOURCE_TARGETS app4
    LUPDATE_TARGET app4_lupdate
    LRELEASE_TARGET app4_lrelease
    TS_FILES myobject_lv.ts myobject_et.ts
    QM_FILES_OUTPUT_VARIABLE qm_files
    LUPDATE_OPTIONS -source-language en_US
    LRELEASE_OPTIONS -compress)
expect_target(app4_lupdate)
expect_target(app4_lrelease)
expect_target(release_translations)
expect_files_in_list(qm_files myobject_lv.qm myobject_et.qm)

# Typical usage of qt_add_translations with a generated resource.
create_app(app5)
qt6_add_translations(app5
    IMMEDIATE_CALL
    TS_FILES myobject_ru.ts
    RESOURCE_PREFIX "/tränslehschns")

# qt_add_translations on a static lib with a generated resource and the default resource prefix.
# Extract the created resource targets.
add_library(staticlib1 STATIC
    myi18nobject.cpp)
target_include_directories(staticlib1
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/some_dir)
target_link_libraries(staticlib1 PRIVATE Qt6::Core)
set(staticlib1_resource_targets "")
qt6_add_translations(staticlib1
    IMMEDIATE_CALL
    SOURCE_TARGETS staticlib1
    TS_FILES myobject_da.ts
    OUTPUT_TARGETS staticlib1_resource_targets)
if("${staticlib1_resource_targets}" STREQUAL "")
    message(FATAL_ERROR "staticlib1_resource_targets is empty.")
endif()

# Explicitly specify SOURCES.
create_app(app6)
qt6_add_translations(app6
    IMMEDIATE_CALL
    SOURCE_TARGETS app6
    TS_FILES myobject_cs.ts
    SOURCES myi18nobject.cpp)

# Explicitly specify SOURCES with target-less qt_add_lupdate.
qt6_add_lupdate(
    TS_FILES myobject_sk.ts
    SOURCES myi18nobject.cpp
)

# Explicitly specify INCLUDE_DIRECTORIES.
create_app(app7)
qt6_add_translations(app7
    IMMEDIATE_CALL
    SOURCE_TARGETS app7
    TS_FILES myobject_nl.ts
    INCLUDE_DIRECTORIES some_dir)

# Specify the output directory of .qm files with qt_add_lrelease.
create_app(app8)
qt6_add_lupdate(
    SOURCE_TARGETS app8
    TS_FILES myobject_ab.ts
)
qt6_add_lrelease(app8
    TS_FILES myobject_ab.ts
    QM_OUTPUT_DIRECTORY translations
    QM_FILES_OUTPUT_VARIABLE qm_files
)
expect_string_value(qm_files "${CMAKE_CURRENT_BINARY_DIR}/translations/myobject_ab.qm")

# Specify the output directory of .qm files with qt_add_lrelease. Override with OUTPUT_LOCATION.
create_app(app9)
qt6_add_lupdate(
    SOURCE_TARGETS app9
    TS_FILES myobject_aa.ts
)
set_source_files_properties(myobject_aa.ts PROPERTIES
    OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations"
)
qt6_add_lrelease(app9
    TS_FILES myobject_aa.ts
    QM_OUTPUT_DIRECTORY blabliblu
    QM_FILES_OUTPUT_VARIABLE qm_files
)
expect_string_value(qm_files "${CMAKE_CURRENT_BINARY_DIR}/translations/myobject_aa.qm")

# Specify the output directory of .qm files with qt_add_translations.
create_app(app10)
qt6_add_translations(app10
    IMMEDIATE_CALL
    SOURCE_TARGETS app10
    TS_FILES myobject_af.ts
    QM_FILES_OUTPUT_VARIABLE qm_files
    QM_OUTPUT_DIRECTORY translations
)
expect_string_value(qm_files "${CMAKE_CURRENT_BINARY_DIR}/translations/myobject_af.qm")

# Specify the output directory of .qm files with qt_add_translations.
create_app(app11)
set_source_files_properties(myobject_ak.ts PROPERTIES
    OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations"
)
qt6_add_translations(app11
    IMMEDIATE_CALL
    SOURCE_TARGETS app11
    TS_FILES myobject_ak.ts
    QM_FILES_OUTPUT_VARIABLE qm_files
    QM_OUTPUT_DIRECTORY blabliblu
)
expect_string_value(qm_files "${CMAKE_CURRENT_BINARY_DIR}/translations/myobject_ak.qm")
