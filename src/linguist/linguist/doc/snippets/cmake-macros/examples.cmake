# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [qt_add_translation]
qt_add_translation(qmFiles helloworld_en.ts helloworld_de.ts)
#! [qt_add_translation]

#! [qt_add_translation_output_location]
set(TS_FILES helloworld_en.ts helloworld_de.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "l10n")
qt_add_translation(qmFiles ${TS_FILES})
#! [qt_add_translation_output_location]

#! [qt_create_translation]
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} helloworld_en.ts helloworld_de.ts)
#! [qt_create_translation]

#! [auto_determine_ts_file_paths]
project(myproject)
cmake_minimum_required(VERSION 3.19)
find_package(Qt6 COMPONENTS Core LinguistTools)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES de fr)

add_subdirectory(libs)
add_subdirectory(apps)

qt_add_translations(TARGETS myapp)
#! [auto_determine_ts_file_paths]

#! [set_output_location_on_ts_file_with_qt_add_translation]
set_source_files_properties(app_en.ts app_de.ts
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt_add_translation(qmFiles app_en.ts app_de.ts)
#! [set_output_location_on_ts_file_with_qt_add_translation]

#! [set_qm_output_directory_with_qt_add_translations]
qt_add_translations(app QM_OUTPUT_DIRECTORY translations)
#! [set_qm_output_directory_with_qt_add_translations]

#! [set_output_location_on_ts_file_with_qt_add_translations]
set_source_files_properties(app_en.ts app_de.ts
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt_add_translations(app)
#! [set_output_location_on_ts_file_with_qt_add_translations]

#! [set_qm_output_directory_with_qt_add_lrelease]
qt_add_lrelease(
    TS_FILES myapp_en.ts myapp_de.ts
    QM_OUTPUT_DIRECTORY translations
)
#! [set_qm_output_directory_with_qt_add_lrelease]

#! [set_output_location_on_ts_file_with_qt_add_lrelease]
set_source_files_properties(app_en.ts app_de.ts
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt_add_lrelease(TS_FILES myapp_en.ts myapp_de.ts)
#! [set_output_location_on_ts_file_with_qt_add_lrelease]

#! [qt_add_lupdate]
qt_add_lupdate(
    SOURCE_TARGETS myapp
    TS_FILES myapp_de.ts
    PLURALS_TS_FILE myapp_en.ts
)
#! [qt_add_lupdate]

#! [qt_add_translations_default]
cmake_minimum_required(VERSION 3.28)
project(frogger)
find_package(Qt6 COMPONENTS OpenGLWidgets)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES de fr)

# The CMake files in the 'src' subdirectory create
# the targets 'frogger_game' and 'frogger_level_editor'.
add_subdirectory(src)

# Add translations to the 'frogger_game' target.
qt_add_translations(frogger_game)
#! [qt_add_translations_default]

#! [qt_lupdate_lrelease]
qt_collect_translation_source_targets(i18n_targets)
qt_add_lupdate(
    SOURCE_TARGETS ${i18n_targets}
    TS_FILES frogger_de.ts frogger_fr.ts)
qt_add_lrelease(
    TS_FILES frogger_de.ts frogger_fr.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
qt_add_resources(frogger_game "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES "${qm_files}"
)
#! [qt_lupdate_lrelease]

#! [qt_add_translations_explicit_source_targets]
qt_add_translations(frogger_game
    SOURCE_TARGETS frogger_game
)
#! [qt_add_translations_explicit_source_targets]

#! [qt_add_translations_resource_prefix]
qt_add_translations(
    TARGETS frogger_game frogger_level_editor
    RESOURCE_PREFIX "/translations"
)
#! [qt_add_translations_resource_prefix]

#! [qt_add_translations_install]
qt_add_translations(
    TARGETS frogger_game frogger_level_editor
    QM_FILES_OUTPUT_VARIABLE qm_files
)
install(FILES ${qm_files} DESTINATION "translations")
#! [qt_add_translations_install]

#! [qt_collect_translation_source_targets]
add_subdirectory(src)         # The actual application is defined here.

# All targets that have been defined up to this point will be in i18n_targets.
qt_collect_translation_source_targets(i18n_targets)
qt_add_lupdate(SOURCE_TARGETS ${i18n_targets})

# No targets from this directory are in i18n_targets.
add_subdirectory(tests)       # Unit tests - we don't want to translate those.
#! [qt_collect_translation_source_targets]

#! [qt_collect_translation_source_targets_deferred]
function(set_up_translations)
    qt_collect_translation_source_targets(i18n_targets)
    qt_add_lupdate(SOURCE_TARGETS ${i18n_targets})
endfunction()

cmake_language(DEFER CALL set_up_translations)

add_subdirectory(src)         # The actual application is defined here.
add_subdirectory(tests)       # Unit tests - we don't want to translate those.
set_property(DIRECTORY tests PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
#! [qt_collect_translation_source_targets_deferred]

#! [exclude sources from i18n]
qt_add_executable(myapp
    main.cpp
    untranslatable.cpp
    3rdparty/sqlite/sqlite3.h
    3rdparty/sqlite/sqlite3.c
    3rdparty/zlib/src/gzlib.c
    3rdparty/zlib/src/zlib.h
)
set_property(TARGET myapp PROPERTY QT_EXCLUDE_SOURCES_FROM_TRANSLATION
    untranslatable.cpp
    3rdparty/*
)
#! [exclude sources from i18n]
