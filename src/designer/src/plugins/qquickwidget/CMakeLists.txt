# Generated from qquickwidget.pro.

#####################################################################
## qquickwidget Plugin:
#####################################################################

add_qt_plugin(qquickwidget
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

add_qt_resource(qquickwidget "qquickwidget_plugin"
    PREFIX
        "/qt-project.org/qquickwidget"
    FILES
        ${qquickwidget_plugin_resource_files}
)

