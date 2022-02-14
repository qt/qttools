cmake_minimum_required(VERSION 3.16)
project(worldtimeclockplugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/plugins/designer")

find_package(Qt6 REQUIRED COMPONENTS Core Gui UiPlugin Widgets)

qt_add_plugin(worldtimeclockplugin)
target_sources(worldtimeclockplugin PRIVATE
    worldtimeclock.cpp worldtimeclock.h
    worldtimeclockplugin.cpp worldtimeclockplugin.h
)

set_target_properties(worldtimeclockplugin PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(worldtimeclockplugin PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UiPlugin
    Qt::Widgets
)

install(TARGETS worldtimeclockplugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
