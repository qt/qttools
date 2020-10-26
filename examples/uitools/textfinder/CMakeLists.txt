# Generated from textfinder.pro.

cmake_minimum_required(VERSION 3.14)
project(textfinder LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/uitools/textfinder")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS UiTools)

qt_add_executable(textfinder
    main.cpp
    textfinder.cpp textfinder.h
)
set_target_properties(textfinder PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(textfinder PUBLIC
    Qt::Core
    Qt::Gui
    Qt::UiTools
    Qt::Widgets
)


# Resources:
set(textfinder_resource_files
    "forms/input.txt"
    "forms/textfinder.ui"
)

qt6_add_resources(textfinder "textfinder"
    PREFIX
        "/"
    FILES
        ${textfinder_resource_files}
)

install(TARGETS textfinder
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
