# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [0]
cmake_minimum_required(VERSION 3.16)
project(calculatorform LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(calculatorform
                  calculatorform.ui main.cpp)

set_target_properties(calculatorform PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(calculatorform  PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
#! [0]
