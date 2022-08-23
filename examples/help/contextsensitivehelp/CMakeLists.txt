# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(contextsensitivehelp LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/help/contextsensitivehelp")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Help Widgets)

qt_add_executable(contextsensitivehelp
    helpbrowser.cpp helpbrowser.h
    main.cpp
    wateringconfigdialog.cpp wateringconfigdialog.h wateringconfigdialog.ui
)

set_target_properties(contextsensitivehelp PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(contextsensitivehelp PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Help
    Qt::Widgets
)

install(TARGETS contextsensitivehelp
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
