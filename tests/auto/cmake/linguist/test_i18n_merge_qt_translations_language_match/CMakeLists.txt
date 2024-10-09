# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Test description:
# - Specify that the project intends to provide a translation for "de".
# - Create fake Qt translations for "de", because qttools cannot access qttranslations in the CI.
# - Pass an existing myapp_de.ts file that has 'language="de_DE"' set.
# - Specify MERGE_QT_TRANSLATIONS to merge our fake Qt translations into myapp's translations.
# - Run the binary 'myapp' that checks internally whether the translation is working.
#   The actual running is done in the CMakeLists.txt one level above this one.
#
# It's expected that we can merge the "de" Qt translations despite seeing "de_DE" in the .ts file.

cmake_minimum_required(VERSION 3.16)
project(test_i18n_merge_qt_translations_language_match)

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES de)

qt_add_executable(myapp
    main.cpp
)
target_compile_definitions(myapp PRIVATE QT_RESTRICTED_CAST_FROM_ASCII)

# Point to the directory that contains the fake translation catalogs.
set(fake_translations_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/fake_translations")
set(QT_INTERNAL_ABSOLUTE_TRANSLATIONS_DIR "${CMAKE_CURRENT_BINARY_DIR}/fake_translations")
file(MAKE_DIRECTORY "${QT_INTERNAL_ABSOLUTE_TRANSLATIONS_DIR}")

# Create the .qm files of our fake translation catalogs.
_qt_internal_get_tool_wrapper_script_path(tool_wrapper)
file(GLOB ts_files "${fake_translations_source_dir}/*.ts")
get_target_property(lrelease_path Qt6::lrelease IMPORTED_LOCATION)
foreach(ts_file IN LISTS ts_files)
    get_filename_component(qm_file "${ts_file}" NAME_WLE)
    string(PREPEND qm_file "${QT_INTERNAL_ABSOLUTE_TRANSLATIONS_DIR}/")
    string(APPEND qm_file ".qm")
    execute_process(
        COMMAND ${tool_wrapper} ${lrelease_path} "${ts_file}" -qm "${qm_file}"
        COMMAND_ECHO STDOUT
        RESULT_VARIABLE exit_code
    )
    if(NOT exit_code EQUAL "0")
        message(FATAL_ERROR "lrelease exited with code ${exit_code}.")
    endif()
endforeach()

qt_add_translations(myapp
    TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/myapp_de.ts"
    NO_GENERATE_PLURALS_TS_FILE
    MERGE_QT_TRANSLATIONS
)

# Additionally, check whether we can extract the language from unusually formatted .ts files.
foreach(ts_file_flavor newlines TSFOO_tag language_after_TS)
    set(ts_file_path "${CMAKE_CURRENT_SOURCE_DIR}/myapp_de_${ts_file_flavor}.ts")
    _qt_internal_read_language_from_ts_file(language "${ts_file_path}")
    if(NOT language STREQUAL "de_DE")
        message(FATAL_ERROR "Could not extract the language from '${ts_file_path}'.")
    endif()
endforeach()
