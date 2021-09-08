# Generated from trollprint.pro.

cmake_minimum_required(VERSION 3.16)
project(trollprint LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/linguist/trollprint")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

qt_add_executable(trollprint
    main.cpp
    mainwindow.cpp mainwindow.h
    printpanel.cpp printpanel.h
)
qt6_add_translations(trollprint
    TS_FILES trollprint_pt.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
install(FILES ${qm_files} DESTINATION ${INSTALL_EXAMPLEDIR})

set_target_properties(trollprint PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(trollprint PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

install(TARGETS trollprint
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
