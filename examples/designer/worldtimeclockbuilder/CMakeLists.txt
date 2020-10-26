# Generated from worldtimeclockbuilder.pro.

cmake_minimum_required(VERSION 3.14)
project(worldtimeclockbuilder LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/designer/worldtimeclockbuilder")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS UiTools)

qt_add_executable(worldtimeclockbuilder
    main.cpp
)
set_target_properties(worldtimeclockbuilder PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(worldtimeclockbuilder PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UiTools
    Qt::Widgets
)


# Resources:
set(worldtimeclockbuilder_resource_files
    "form.ui"
)

qt6_add_resources(worldtimeclockbuilder "worldtimeclockbuilder"
    PREFIX
        "/forms"
    FILES
        ${worldtimeclockbuilder_resource_files}
)

install(TARGETS worldtimeclockbuilder
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
