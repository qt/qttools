# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(i18n LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/tools/i18n")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

qt_standard_project_setup(
    I18N_TRANSLATED_LANGUAGES ar cs de el en eo fr it ja ko nb ru sv zh
)

qt_add_executable(i18n
    languagechooser.cpp languagechooser.h
    main.cpp
    mainwindow.cpp mainwindow.h
)

set_target_properties(i18n PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(i18n PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

qt_add_translations(i18n
    TS_OUTPUT_DIRECTORY translations
)

install(TARGETS i18n
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
