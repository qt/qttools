# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(multipleinheritance LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(multipleinheritance
                  imagedialog.cpp  imagedialog.h  imagedialog.ui  main.cpp)

set_target_properties(multipleinheritance PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(multipleinheritance  PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
