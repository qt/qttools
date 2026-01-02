# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lrelease-pro Tool:
#####################################################################

qt_get_tool_target_name(target_name lrelease-pro)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Compiler for QMake Projects"
    TOOLS_TARGET Linguist
    USER_FACING
    SOURCES
        ../shared/qm.cpp
        ../shared/ts.cpp
        ../shared/validator.cpp ../shared/validator.h
        ../shared/releasehelper.cpp ../shared/releasehelper.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        LINGUIST_CONSOLE_APPLICATION
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        LinguistProject
)
qt_internal_return_unless_building_tools()
