# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES de fr)

add_subdirectory(libs)
add_subdirectory(apps)

qt_add_translations(TARGETS myapp)
#! [auto_determine_ts_file_paths]

#! [set_output_location_on_ts_file]
set_source_files_properties(app_en.ts app_de.ts
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
#! [set_output_location_on_ts_file]

#! [qt_add_lupdate]
qt_add_lupdate(
    TS_FILES myapp_de.ts
    PLURALS_TS_FILE myapp_en.ts
)
#! [qt_add_lupdate]

#! [qt_add_lrelease_install]
qt_add_lrelease(myapp
    TS_FILES myapp_de.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
install(FILES ${qm_files} DESTINATION "translations")
#! [qt_add_lrelease_install]

#! [qt_add_translations_default]
qt_add_translations(super_calc TS_FILES super_calc_de.ts)
#! [qt_add_translations_default]

#! [qt_lupdate_lrelease]
qt_collect_translation_source_targets(i18n_targets)
qt_add_lupdate(
    TARGETS ${i18n_targets}
    TS_FILES super_calc_de.ts)
qt_add_lrelease(
    TS_FILES super_calc_de.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
qt_add_resources(super_calc "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES "${qm_files}")
#! [qt_lupdate_lrelease]

#! [qt_add_translations_resource_prefix]
qt_add_translations(
    TARGETS frogger_game frogger_level_editor
    TS_FILES frogger_game_no.ts
    RESOURCE_PREFIX "/translations")
#! [qt_add_translations_resource_prefix]

#! [qt_add_translations_install]
qt_add_translations(business_logic
    TS_FILES myapp_fi.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
install(FILES ${qm_files} DESTINATION "translations")
#! [qt_add_translations_install]

#! [qt_collect_translation_source_targets]
add_subdirectory(src)          # the actual application is defined here

qt_collect_translation_source_targets(i18n_targets)
qt_add_lupdate(TARGETS ${i18n_targets})

add_subdirectory(tests)        # unit tests - we don't want to translate those
#! [qt_collect_translation_source_targets]

#! [exclude sources from i18n]
qt_add_executable(myapp
    main.cpp
    untranslatable.cpp
    3rdparty/sqlite3.h
    3rdparty/sqlite3.c
)
set_property(TARGET myapp PROPERTY QT_EXCLUDE_SOURCES_FROM_TRANSLATION
    untranslatable.cpp
    3rdparty/*
)
#! [exclude sources from i18n]
