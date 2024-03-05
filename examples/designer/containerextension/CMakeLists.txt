# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(containerextension LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

#! [0]
find_package(Qt6 REQUIRED COMPONENTS Core Designer Gui Widgets)

qt_add_plugin(containerextension)
#! [0]

#! [1]
target_sources(containerextension PRIVATE
    multipagewidget.cpp multipagewidget.h
    multipagewidgetcontainerextension.cpp multipagewidgetcontainerextension.h
    multipagewidgetextensionfactory.cpp multipagewidgetextensionfactory.h
    multipagewidgetplugin.cpp multipagewidgetplugin.h
)
#! [1]

set_target_properties(containerextension PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

#! [2]
target_link_libraries(containerextension PUBLIC
    Qt::Core
    Qt::Designer
    Qt::Gui
    Qt::Widgets
)
#! [2]

if(QT6_INSTALL_PREFIX)
#! [3]
   set(INSTALL_EXAMPLEDIR "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_PLUGINS}/designer")
#! [3]
else()
   if(NOT DEFINED INSTALL_EXAMPLESDIR)
       set(INSTALL_EXAMPLESDIR "examples")
   endif()
   set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/plugins/designer")
endif()

#! [4]
install(TARGETS containerextension
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
#! [4]
