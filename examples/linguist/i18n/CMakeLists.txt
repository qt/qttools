# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(i18n LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/tools/i18n")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

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

qt_add_translations(i18n TS_FILES
    translations/i18n_ar.ts
    translations/i18n_cs.ts
    translations/i18n_de.ts
    translations/i18n_el.ts
    translations/i18n_en.ts
    translations/i18n_eo.ts
    translations/i18n_fr.ts
    translations/i18n_it.ts
    translations/i18n_ja.ts
    translations/i18n_ko.ts
    translations/i18n_nb.ts
    translations/i18n_ru.ts
    translations/i18n_sv.ts
    translations/i18n_zh.ts
)

install(TARGETS i18n
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
