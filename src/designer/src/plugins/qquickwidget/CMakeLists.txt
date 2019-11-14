# Generated from qquickwidget.pro.

#####################################################################
## qquickwidget Plugin:
#####################################################################

qt_add_plugin(qquickwidget
    TYPE designer
    CLASS_NAME QQuickWidgetPlugin
    SOURCES
        qquickwidget_plugin.cpp qquickwidget_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::QuickWidgets
        Qt::Widgets
)

# Resources:
set(qquickwidget_plugin_resource_files
    "images/qquickwidget.png"
)

qt_add_resource(qquickwidget "qquickwidget_plugin"
    PREFIX
        "/qt-project.org/qquickwidget"
    FILES
        ${qquickwidget_plugin_resource_files}
)

