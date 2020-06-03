# Generated from containerextension.pro.

cmake_minimum_required(VERSION 3.14)
project(containerextension LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "plugins/designer") # special case

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Designer)

# special case begin
qt_add_plugin(containerextension)
target_sources(containerextension PRIVATE
    multipagewidget.cpp multipagewidget.h
    multipagewidgetcontainerextension.cpp multipagewidgetcontainerextension.h
    multipagewidgetextensionfactory.cpp multipagewidgetextensionfactory.h
    multipagewidgetplugin.cpp multipagewidgetplugin.h
)
# special case end
target_link_libraries(containerextension PUBLIC
    Qt::Core
    Qt::Designer
    Qt::Gui
    Qt::Widgets
)

install(TARGETS containerextension
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
