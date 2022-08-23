# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_uiplugin_via_designer)

find_package(Qt6 COMPONENTS Widgets Xml Designer REQUIRED)

set(CMAKE_AUTOMOC ON)


add_definitions(-DTEST_UIPLUGIN_USAGE_REQUIREMENTS=1)

add_library(my_uiplugin_via_designer my_designer_plugin.cpp)
target_link_libraries(my_uiplugin_via_designer Qt::Designer)
