# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(localizedClockSwitchLocale LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()
set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/demos/localizedclock-switchlocale")

find_package(Qt6 REQUIRED COMPONENTS Core Linguist Qml Quick)

qt_standard_project_setup(REQUIRES 6.8
    I18N_TRANSLATED_LANGUAGES de ar ko zh ja fr it es pt)

qt_add_executable(localizedClockSwitchLocale
    main.cpp
)

qt_add_translations(localizedClockSwitchLocale
    TS_FILE_BASE i18n/clock
    RESOURCE_PREFIX i18n
)

qt_add_qml_module(localizedClockSwitchLocale
    URI qtexamples.localizedclockswitchlocale
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        translatormanager.h translatormanager.cpp
    RESOURCES
        "globe.png"
)

target_link_libraries(localizedClockSwitchLocale PRIVATE
    Qt6::Quick
)

target_compile_definitions(localizedClockSwitchLocale PRIVATE
    QT_NO_CAST_FROM_ASCII
)

set_target_properties(localizedClockSwitchLocale PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)


install(TARGETS localizedClockSwitchLocale
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
