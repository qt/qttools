# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lupdate-pro Tool:
#####################################################################

qt_get_tool_target_name(target_name lupdate-pro)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Update Tool for QMake Projects"
    TOOLS_TARGET Linguist
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        ../shared/runqttool.cpp ../shared/runqttool.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
)
qt_internal_return_unless_building_tools()
