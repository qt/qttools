# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(localizedClockIdBased LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()
set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/demos/localizedClockIdBased")

find_package(Qt6 REQUIRED COMPONENTS Core Linguist Qml Quick Widgets)

qt_standard_project_setup(REQUIRES 6.8
    I18N_TRANSLATED_LANGUAGES de ar ko zh ja fr it es pt en)

qt_add_executable(localizedClockIdBased
    main.cpp
)

qt_add_translations(localizedClockIdBased
    TS_FILE_BASE i18n/clock
    MERGE_QT_TRANSLATIONS
    QT_TRANSLATION_CATALOGS qtbase
    RESOURCE_PREFIX i18n
)

qt_add_qml_module(localizedClockIdBased
    URI qtexamples.localizedclock
    VERSION 1.0
    QML_FILES
        Main.qml
    RESOURCES dialog.ui
    SOURCES
        timezonemanager.h timezonemanager.cpp
        dialog.h dialog.cpp
)

target_compile_definitions(localizedClockIdBased PRIVATE
    QT_NO_CAST_FROM_ASCII
)

target_link_libraries(localizedClockIdBased PRIVATE
    Qt6::Quick Qt6::Widgets
)

set_target_properties(localizedClockIdBased PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

install(TARGETS localizedClockIdBased
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

