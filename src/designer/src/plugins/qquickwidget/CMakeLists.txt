# Generated from qquickwidget.pro.

#####################################################################
## QQuickWidgetPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QQuickWidgetPlugin
    OUTPUT_NAME qquickwidget
    TYPE designer
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

qt_internal_add_resource(QQuickWidgetPlugin "qquickwidget_plugin"
    PREFIX
        "/qt-project.org/qquickwidget"
    FILES
        ${qquickwidget_plugin_resource_files}
)

