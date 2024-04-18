# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_i18n_filter_autogen_files)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 COMPONENTS Widgets LinguistTools)
qt_standard_project_setup()
add_subdirectory(app1)
add_subdirectory(app2)

set(ts_file "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_de.ts")
qt_add_lupdate(
    TS_FILES "${ts_file}"
    SOURCE_TARGETS app1 app2
    LUPDATE_TARGET update_ts_files
    NO_GLOBAL_TARGET
)

# Make sure that we build the app before creating the .ts file.
# We want to have AUTOMOC and AUTOUIC files generated.
add_dependencies(update_ts_files app1 app2)

# Update the .ts files by default and check the content of the updated .ts file.
add_custom_target(force_ts_update ALL
    COMMENT "Checking .ts file contents..."
    COMMAND "${CMAKE_COMMAND}" -DTS_FILE="${ts_file}"
                               -P "${CMAKE_CURRENT_SOURCE_DIR}/check_ts_file.cmake"
)
add_dependencies(force_ts_update update_ts_files)
