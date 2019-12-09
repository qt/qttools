# Generated from view3d.pro.

#####################################################################
## view3d Plugin:
#####################################################################

qt_add_plugin(view3d
    TYPE designer
    CLASS_NAME QView3DPlugin
    SOURCES
        view3d.cpp view3d.h
        view3d_global.h
        view3d_plugin.cpp view3d_plugin.h
        view3d_tool.cpp view3d_tool.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::OpenGL
        Qt::Widgets
)
