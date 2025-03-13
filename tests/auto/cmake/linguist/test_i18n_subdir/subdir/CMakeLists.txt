# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_executable(app1
    main.cpp)
target_link_libraries(app1 PRIVATE Qt::Gui)
qt_add_translations(app1
    TS_FILES app1_de.ts
    NO_GENERATE_PLURALS_TS_FILE
)

include(../../build_time_checks.cmake)
check_file_is_built("${CMAKE_CURRENT_BINARY_DIR}/app1_de.qm")
