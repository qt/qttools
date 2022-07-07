# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(remotecontrol LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/assistant/remotecontrol")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(remotecontrol
    main.cpp
    remotecontrol.cpp remotecontrol.h remotecontrol.ui
)

set_target_properties(remotecontrol PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(remotecontrol PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

# Resources:
set(remotecontrol_resource_files
    "enter.png"
)

qt6_add_resources(remotecontrol "remotecontrol"
    PREFIX
        "/remotecontrol"
    FILES
        ${remotecontrol_resource_files}
)

install(TARGETS remotecontrol
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
