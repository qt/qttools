# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## UiPlugin Module:
#####################################################################

qt_internal_add_module(UiPlugin
    NO_PRIVATE_MODULE
    HEADER_MODULE
    QMAKE_MODULE_CONFIG designer_defines
    SOURCES
        customwidget.h
        qdesignerexportwidget.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

set(is_plugin "$<TARGET_PROPERTY:QT_PLUGIN_CLASS_NAME>")
target_compile_definitions(
    UiPlugin
    INTERFACE
        $<$<BOOL:${is_plugin}>:QDESIGNER_EXPORT_WIDGETS>
)
