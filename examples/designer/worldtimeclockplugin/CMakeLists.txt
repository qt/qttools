cmake_minimum_required(VERSION 3.16)
project(worldtimeclockplugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

#! [0]
find_package(Qt6 REQUIRED COMPONENTS Core Gui UiPlugin Widgets)

qt_add_plugin(worldtimeclockplugin)
#! [0]

#! [1]
target_sources(worldtimeclockplugin PRIVATE
    worldtimeclock.cpp worldtimeclock.h
    worldtimeclockplugin.cpp worldtimeclockplugin.h
)
#! [1]

set_target_properties(worldtimeclockplugin PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

#! [2]
target_link_libraries(worldtimeclockplugin PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UiPlugin
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
install(TARGETS worldtimeclockplugin
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
#! [4]
