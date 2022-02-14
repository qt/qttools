cmake_minimum_required(VERSION 3.16)
project(calculatorform LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/designer/calculatorform")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(calculatorform
    calculatorform.cpp calculatorform.h calculatorform.ui
    main.cpp
)

set_target_properties(calculatorform PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(calculatorform PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

install(TARGETS calculatorform
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
