# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
qt_add_executable(app2
    main.cpp
    mainwindow.ui
)
target_link_libraries(app2 PRIVATE Qt6::Widgets)
set_target_properties(app2 PROPERTIES
    AUTOGEN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/wossname_autogen"
)
