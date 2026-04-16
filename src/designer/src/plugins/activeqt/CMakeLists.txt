# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QAxWidgetPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QAxWidgetPlugin
    OUTPUT_NAME qaxwidget
    PLUGIN_TYPE designer
    SOURCES
        qaxwidgetextrainfo.cpp qaxwidgetextrainfo.h
        qaxwidgetplugin.cpp qaxwidgetplugin.h
        qaxwidgetpropertysheet.cpp qaxwidgetpropertysheet.h
        qaxwidgettaskmenu.cpp qaxwidgettaskmenu.h
        qdesigneraxwidget.cpp qdesigneraxwidget.h
    LIBRARIES
        Qt::AxContainer
        Qt::Core
        Qt::Designer
        Qt::DesignerPrivate
        Qt::Gui
        Qt::Widgets
)

# OTHER_FILES = "activeqt.json"
