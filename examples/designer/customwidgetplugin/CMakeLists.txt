cmake_minimum_required(VERSION 3.16)
project(customwidgetplugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/plugins/designer")

find_package(Qt6 REQUIRED COMPONENTS Core Gui UiPlugin Widgets)

qt_add_plugin(customwidgetplugin)
target_sources(customwidgetplugin PRIVATE
    analogclock.cpp analogclock.h
    customwidgetplugin.cpp customwidgetplugin.h
)

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
