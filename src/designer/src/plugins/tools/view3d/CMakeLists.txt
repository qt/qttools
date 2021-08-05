# Generated from view3d.pro.

#####################################################################
## QView3DPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QView3DPlugin
    OUTPUT_NAME view3d
    PLUGIN_TYPE designer
    SOURCES
        view3d.cpp view3d.h
        view3d_global.h
        view3d_plugin.cpp view3d_plugin.h
        view3d_tool.cpp view3d_tool.h
    LIBRARIES
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::OpenGL
        Qt::Widgets
)
