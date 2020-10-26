# Generated from taskmenuextension.pro.

cmake_minimum_required(VERSION 3.14)
project(taskmenuextension LANGUAGES CXX)

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
find_package(Qt6 COMPONENTS Designer)

# special case begin
qt_add_plugin(taskmenuextension)
target_sources(taskmenuextension PRIVATE
    tictactoe.cpp tictactoe.h
    tictactoedialog.cpp tictactoedialog.h
    tictactoeplugin.cpp tictactoeplugin.h
    tictactoetaskmenu.cpp tictactoetaskmenu.h
)
# special case end
set_target_properties(taskmenuextension PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(taskmenuextension PUBLIC
    Qt::Core
    Qt::Designer
    Qt::Gui
    Qt::Widgets
)

install(TARGETS taskmenuextension
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
