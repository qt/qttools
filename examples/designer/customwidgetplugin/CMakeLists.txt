# Generated from customwidgetplugin.pro.

cmake_minimum_required(VERSION 3.14)
project(customwidgetplugin LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/plugins/designer") # special case

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS UiPlugin)

# special case begin
qt_add_plugin(customwidgetplugin)
target_sources(customwidgetplugin PRIVATE
    analogclock.cpp analogclock.h
    customwidgetplugin.cpp customwidgetplugin.h
)
# special case end
set_target_properties(customwidgetplugin PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(customwidgetplugin PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UiPlugin
    Qt::Widgets
)

install(TARGETS customwidgetplugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
