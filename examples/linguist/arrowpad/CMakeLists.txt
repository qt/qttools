# Generated from arrowpad.pro.

cmake_minimum_required(VERSION 3.16)
project(arrowpad LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/linguist/arrowpad")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
#! [0]
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
#! [0]

qt_add_executable(arrowpad
    arrowpad.cpp arrowpad.h
    main.cpp
    mainwindow.cpp mainwindow.h
)
#! [1]
qt6_add_translations(arrowpad
    TS_FILES arrowpad_fr.ts arrowpad_nl.ts
    QM_FILES_OUTPUT_VARIABLE qm_files)
install(FILES ${qm_files} DESTINATION ${INSTALL_EXAMPLEDIR})
#! [1]
set_target_properties(arrowpad PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(arrowpad PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

install(TARGETS arrowpad
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
