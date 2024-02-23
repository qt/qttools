# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_i18n_exclusion)

# Set up the project structure.
find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools)
qt_standard_project_setup()
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(tests)

# Generate a fresh, empty .ts file in the build directory.
set(ts_file "${CMAKE_CURRENT_BINARY_DIR}/test_i18n_exclusion_de.ts")
configure_file(test_i18n_exclusion_de.ts.in "${ts_file}")

# Set up the 'update_translations' target.
qt_collect_translation_source_targets(i18n_targets)
qt_add_lupdate(
    SOURCE_TARGETS ${i18n_targets}
    TS_FILES "${ts_file}"
)

# Build the 'update_translations' target by default and check the content of the updated .ts file.
add_custom_target(force_ts_update ALL
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/check_ts_file.cmake"
)
add_dependencies(force_ts_update update_translations)
