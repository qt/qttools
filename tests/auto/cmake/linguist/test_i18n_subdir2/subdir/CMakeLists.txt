# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_executable(app1 main.cpp)
target_link_libraries(app1 PRIVATE Qt::Gui)
qt_add_translations(app1
    NO_GENERATE_PLURALS_TS_FILE
    TS_FILE_BASE app1
    QM_FILES_OUTPUT_VARIABLE qm_files    # unused, but turns off automatic resource generation
)

# Use qt_add_resources to check whether the .qm file was created where we expected it.
qt_add_resources(app1 "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES "${CMAKE_CURRENT_BINARY_DIR}/app1_fr.qm"
)
