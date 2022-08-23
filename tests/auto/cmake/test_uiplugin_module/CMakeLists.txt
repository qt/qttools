# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_uiplugin_module)

find_package(Qt6 COMPONENTS Widgets UiPlugin REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(my_designer_plugin my_designer_plugin.cpp)
target_link_libraries(my_designer_plugin Qt::UiPlugin)
