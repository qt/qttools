# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Test description:
# - Specify that the project intends to provide a translation for "de".
# - Create fake Qt translations for "de", because qttools cannot access qttranslations in the CI.
#   The translation catalogs provided are "qtbase" and "greenphone".
# - Auto-generate a myapp_de.ts file, which will have 'language="de"' set.
# - Specify MERGE_QT_TRANSLATIONS to merge our fake Qt translations into myapp's translations.
# - Specify that we want to use the catalogs "qtbase" and "greenphone".
# - Run the binary 'myapp' that checks internally whether the translation is working.
#   The actual running is done in the CMakeLists.txt one level above this one.

cmake_minimum_required(VERSION 3.16)
project(test_i18n_merge_qt_translations)

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
    TS_FILE_BASE myapp
    NO_GENERATE_PLURALS_TS_FILE
    MERGE_QT_TRANSLATIONS
    QT_TRANSLATION_CATALOGS qtbase greenphone
)
