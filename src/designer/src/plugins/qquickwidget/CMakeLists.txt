# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QQuickWidgetPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QQuickWidgetPlugin
    OUTPUT_NAME qquickwidget
    PLUGIN_TYPE designer
    SOURCES
        qquickwidget_plugin.cpp qquickwidget_plugin.h
    LIBRARIES
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
