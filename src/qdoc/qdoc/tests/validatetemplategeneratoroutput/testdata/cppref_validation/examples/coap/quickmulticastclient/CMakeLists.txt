# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(quickmulticastclient LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/coap/quickmulticastclient")

find_package(Qt6 REQUIRED COMPONENTS Coap Core Gui Qml Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(quickmulticastclient
    main.cpp
)

set_target_properties(quickmulticastclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(quickmulticastclient PRIVATE
    Qt6::Coap
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

qt_add_qml_module(quickmulticastclient
    URI CoapClientModule
    VERSION 1.0
    SOURCES
        qmlcoapmulticastclient.cpp qmlcoapmulticastclient.h
    QML_FILES
        Main.qml
)

install(TARGETS quickmulticastclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
