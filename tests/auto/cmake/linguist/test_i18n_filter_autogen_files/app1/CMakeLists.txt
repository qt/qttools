# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
qt_add_executable(app1
    main.cpp
    mainwindow.ui
)
target_link_libraries(app1 PRIVATE Qt6::Widgets)
