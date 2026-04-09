# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(simplecoapclient LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/coap/simplecoapclient")

find_package(Qt6 REQUIRED COMPONENTS Coap Core Gui Network Widgets)

qt_standard_project_setup()

qt_add_executable(simplecoapclient
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    optiondialog.cpp optiondialog.h optiondialog.ui
)

set_target_properties(simplecoapclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(simplecoapclient PRIVATE
    Qt6::Coap
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

install(TARGETS simplecoapclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
