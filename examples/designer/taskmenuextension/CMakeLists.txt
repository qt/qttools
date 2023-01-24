# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(taskmenuextension LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

#! [0]
find_package(Qt6 REQUIRED COMPONENTS Core Designer Gui Widgets)

qt_add_plugin(taskmenuextension)
#! [0]

#! [1]
target_sources(taskmenuextension PRIVATE
    tictactoe.cpp tictactoe.h
    tictactoedialog.cpp tictactoedialog.h
    tictactoeplugin.cpp tictactoeplugin.h
    tictactoetaskmenu.cpp tictactoetaskmenu.h
)
#! [1]

set_target_properties(taskmenuextension PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

#! [2]
target_link_libraries(taskmenuextension PUBLIC
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
install(TARGETS taskmenuextension
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
#! [4]
